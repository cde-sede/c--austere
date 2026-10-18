#!/usr/bin/env python3
"""Generate Markdown or HTML reference docs from /* @DOC */ comments.

Usage: autodoc.py [lib] [-o DIR] [-f md|html] [--check] [--missing] [--clean | --fclean]
       autodoc.py --bench [NAME...] [--cxx CXX,...]
Defaults: <repo>/src/lib -> <repo>/doc (md) or <repo>/doc/html (html).

--bench runs the benchmarks a @DOC quotes between `--BENCH bench/NAME.cpp [SECTION]` and
`BENCH--` lines and rewrites those lines in the sources (see bench_tables.py), then
generates the docs; NAME... limits it to those benches, --cxx sets the compilers
(default clang++,g++), one column each. The marker lines never reach the docs.

/* @DOC text */ documents the declaration right below it: namespace, class, struct,
union, enum, concept, alias, function, variable, #define. Template headers and
requires-clauses belong to the declaration. Inside a class it documents a member.
A @DOC before the include guard documents the whole header.

Public ca entities without a @DOC are listed per header (impl internals skipped).
--missing prints them; --check exits 1 on orphan @DOC comments or missing docs.
--clean leaves nothing to remove but bytecode; --fclean also removes the pages (both formats
at their default places unless -f or -o is given) and their directories once empty (see
cleanup.py).
"""

import argparse
import bisect
import html
import json
import os
import re
import subprocess
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import bench_tables  # noqa: E402
import cleanup  # noqa: E402
import norm  # noqa: E402

DOC = re.compile(r'/\*\s*@DOC\b(.*?)\*/', re.S)
REQUIRES_EXPR = re.compile(r'(?:=|&&|\|\||\(|!|,|\brequires)\s*requires\s*(?:\([^{}]*\))?\s*$')
SCOPE_PUNCT = re.compile(r'[{};]')
NAMESPACE_HEAD = re.compile(r'\bnamespace\s+([\w:]+)\s*$')
CLASS_HEAD = re.compile(r'\b(?:class|struct|union)\b\s*(?:\[\[.*?\]\]\s*)*([A-Za-z_]\w*)[^;{}]*$')
FUNCTION_TAIL = re.compile(r'\)\s*(?:const|noexcept|->)')
DEFINE = re.compile(r'#\s*define\b')
LAYER = re.compile(r'\d+-')
FACADE = 'ca'
HTML_LIKE = re.compile(r'<(?=/?[A-Za-z][A-Za-z0-9-]*(?:[\s/>]|$)|[A-Za-z][\w+.-]*:)|&(?=#?\w+;)')
REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


class Entry:
	def __init__(self, kind, name, signature, text, scope, line):
		self.kind = kind
		self.name = name
		self.signature = signature
		self.text = text
		self.scope = scope
		self.line = line
		self.qual = ''
		self.params = ''
		self.sources = []


class Header:
	def __init__(self, rel, key):
		self.rel = rel
		self.key = key
		self.files = [key]
		self.doc = ''
		self.entries = []
		self.entities = set()
		self.orphans = []

	def documented(self):
		return {e.name for e in self.entries if not e.scope}

	def summary(self):
		return summary_of(self.doc or next((e.text for e in self.entries if not e.scope and e.text), ''))

	def missing(self):
		return sorted(self.entities - self.documented())


def doc_text(body):
	lines = [l.strip() for l in body.strip('\n').split('\n')]
	blocks, para, rows = [], [], []
	for l in lines + ['']:
		if bench_tables.is_marker(l):
			continue
		if l.startswith(('- ', '|')):
			if para:
				blocks.append(' '.join(para))
				para = []
			rows.append(l)
		elif l and rows and rows[-1].startswith('- '):
			rows[-1] += ' ' + l
		else:
			if rows:
				blocks.append('\n'.join(rows))
				rows = []
			if l:
				para.append(l)
			elif para:
				blocks.append(' '.join(para))
				para = []
	return '\n\n'.join(blocks)


def mask_comments(text, comments):
	chars = list(text)
	for off, body, _ in comments:
		for k in range(off, off + len(body)):
			if chars[k] != '\n':
				chars[k] = ' '
	return ''.join(chars)


class Scopes:
	def __init__(self, code):
		self.code = code
		self.stack = []
		self.pos = 0
		self.head = 0

	def at(self, upto):
		code = self.code
		for m in SCOPE_PUNCT.finditer(code, self.pos, max(upto, self.pos)):
			i = m.start()
			if code[i] == '{':
				head = code[self.head:i]
				n = NAMESPACE_HEAD.search(head)
				c = CLASS_HEAD.search(head)
				if n:
					self.stack.append(('namespace', n.group(1)))
				elif c and not FUNCTION_TAIL.search(head, c.start()):
					self.stack.append(('class', c.group(1)))
				else:
					self.stack.append(('block', ''))
			elif code[i] == '}' and self.stack:
				self.stack.pop()
			self.head = i + 1
		self.pos = max(upto, self.pos)
		return list(self.stack)


def declaration_end(code, i):
	start = i
	depth = 0
	n = len(code)
	while i < n:
		ch = code[i]
		if ch in '([':
			depth += 1
		elif ch in ')]':
			depth -= 1
		elif depth == 0 and ch == ';':
			return i + 1
		elif depth == 0 and ch == '{':
			if REQUIRES_EXPR.search(code, start, i):
				k = norm.match_close(code, i, '{', '}')
				if k < 0:
					return n
				i = k + 1
				continue
			return i
		i += 1
	return n


def skip_preprocessor(code, i):
	while True:
		j = i
		while j < len(code) and code[j] in ' \t\n':
			j += 1
		if j < len(code) and code[j] == '#' and not DEFINE.match(code, j):
			e = code.find('\n', j)
			i = len(code) if e < 0 else e + 1
			continue
		return j


def dedent(sig):
	lines = [l.rstrip() for l in sig.replace('\t', '    ').split('\n')]
	lines = [l for l in lines if l.strip()]
	if not lines:
		return ''
	pad = min(len(l) - len(l.lstrip()) for l in lines)
	return '\n'.join(l[pad:] for l in lines)


def classify(sig):
	flat = ' '.join(sig.split())
	if flat.startswith('#'):
		m = re.match(r'#\s*define\s+(\w+)', flat)
		return 'macro', m.group(1) if m else '?'
	_, rest = norm.strip_templates(flat)
	rest = norm.strip_attrs(rest)
	m = re.match(r'(?:inline\s+)?namespace\s+([\w:]+)', rest)
	if m:
		return 'namespace', m.group(1)
	m = re.match(r'enum\s+(?:class\s+|struct\s+)?(\w+)', rest)
	if m:
		return 'enum', m.group(1)
	m = re.match(r'(class|struct|union)\b\s*(?:\[\[.*?\]\]\s*)*(\w+)', rest)
	if m:
		return m.group(1), m.group(2)
	m = re.match(r'concept\s+(\w+)', rest)
	if m:
		return 'concept', m.group(1)
	m = re.match(r'using\s+(\w+)\s*=', rest)
	if m:
		return 'alias', m.group(1)
	fn = norm.split_function(rest) if '(' in rest else None
	if fn and (re.search(r'\boperator\b', rest) or '=' not in rest[:rest.find('(')]):
		name = fn.name.split('::')[-1] if 'operator' not in fn.name else fn.name[fn.name.find('operator'):]
		return 'function', name
	m = re.search(r'([A-Za-z_]\w*)\s*(?:=|\{|;|$)', re.split(r'=|\{', rest)[0] + ';')
	return 'variable', m.group(1) if m else '?'


def entities_of(path, text):
	return norm.entities(path, text)


def line_starts(text):
	starts = [0]
	for m in re.finditer('\n', text):
		starts.append(m.end())
	return starts


def line_of(starts, off):
	return bisect.bisect_right(starts, off)


def body_open(code, start, brace):
	n = len(code)
	if norm.init_colon(' '.join(code[start:brace].split())):
		while code[:brace].rstrip()[-1:] not in (')', '}'):
			k = norm.match_close(code, brace, '{', '}')
			if k < 0:
				return brace
			depth, p = 0, k + 1
			while p < n and not (code[p] == '{' and depth == 0):
				depth += code[p] in '(['
				depth -= code[p] in ')]'
				p += 1
			if p >= n:
				return brace
			brace = p
	return brace


def body_close(code, start, brace):
	k = norm.match_close(code, body_open(code, start, brace), '{', '}')
	return len(code) - 1 if k < 0 else k


def qual_name(name):
	name = name.replace(' ', '').lstrip(':')
	k = name.find('operator')
	head, op = (name, '') if k < 0 else (name[:k], name[k:])
	while True:
		stripped = re.sub(r'<[^<>]*>', '', head)
		if stripped == head:
			return head + op
		head = stripped


def param_key(params):
	parts, depth, cur = [], 0, ''
	for ch in params:
		depth += ch in '(<[{'
		depth -= ch in ')>]}'
		if ch == ',' and depth == 0:
			parts.append(cur)
			cur = ''
		else:
			cur += ch
	keys = []
	for p in parts + [cur]:
		p = norm.strip_attrs(re.split(r'(?<![=!<>])=(?!=)', p)[0])
		toks = [t for t in re.findall(r'\w+|::|\.\.\.|[^\s\w]', p) if t not in ('const', 'volatile')]
		if len(toks) > 1 and re.match(r'[A-Za-z_]\w*$', toks[-1]) and toks[-2] != '::':
			toks.pop()
		keys.append(''.join(toks))
	return ','.join(k for k in keys if k and k != 'void')


def function_of(sig):
	_, rest = norm.strip_templates(' '.join(sig.split()))
	rest = norm.strip_attrs(rest)
	return norm.split_function(rest) if '(' in rest else None


def definitions(key, text, defs):
	code, _ = norm.lex(text)
	starts = line_starts(text)
	stack, i, n = [], 0, len(code)
	while True:
		while i < n and code[i] in ' \t\n':
			i += 1
		if i < n and code[i] == '#':
			while True:
				nl = code.find('\n', i)
				nl = n if nl < 0 else nl
				cont = code[i:nl].rstrip().endswith('\\')
				i = nl + 1
				if not cont:
					break
			continue
		if i >= n:
			return
		if code[i] in '};':
			if code[i] == '}' and stack:
				stack.pop()
			i += 1
			continue
		e = declaration_end(code, i)
		if e >= n:
			return
		if code[e - 1] == ';':
			i = e
			continue
		head = code[i:e]
		ns = re.match(r'\s*(?:inline\s+)?namespace\s*([\w:]*)\s*$', head)
		if ns or re.match(r'\s*extern\s+"_*"\s*$', head):
			stack.append(ns.group(1) if ns else '')
			i = e + 1
			continue
		close = body_close(code, i, e)
		fn = None if CLASS_HEAD.search(head) and '(' not in head else function_of(head)
		if fn:
			qual = qual_name('::'.join([s for s in stack if s] + [fn.name]))
			defs.setdefault(qual, []).append((key, line_of(starts, i), line_of(starts, close), param_key(fn.params)))
		i = close + 1


def resolve_sources(layers, defs):
	for _, headers in layers:
		for h in headers:
			for e in h.entries:
				if e.kind != 'function' or not e.qual:
					continue
				own = {(k, first) for k, first, _ in e.sources}
				cands = [d for d in defs.get(e.qual, []) if (d[0], d[1]) not in own]
				exact = [d for d in cands if d[3] == e.params]
				e.sources += [(k, first, last) for k, first, last, _ in (exact or ([] if e.sources else cands))]


def parse(path, rel, key):
	with open(path, encoding='utf-8') as fh:
		text = fh.read()
	code, comments = norm.lex(text)
	clean = mask_comments(text, comments)
	starts = line_starts(text)
	h = Header(rel, key)
	h.entities = entities_of(path, text)
	guard = re.search(r'^[ \t]*#[ \t]*ifndef\b', code, re.M)
	guard_at = guard.start() if guard else 0
	scopes = Scopes(code)
	line, counted = 1, 0
	for m in DOC.finditer(text):
		if code[m.start()] != ' ':
			continue
		body = doc_text(m.group(1))
		line += text.count('\n', counted, m.start())
		counted = m.start()
		if m.start() < guard_at:
			h.doc = body
			continue
		start = skip_preprocessor(code, m.end())
		if start >= len(code) or code[start] in '}':
			h.orphans.append(line)
			continue
		if code[start] == '#':
			e = start
			while True:
				nl = code.find('\n', e)
				nl = len(code) if nl < 0 else nl
				if not code[e:nl].rstrip().endswith('\\'):
					break
				e = nl + 1
			end = nl
		else:
			end = declaration_end(code, start)
			if end < len(code) and code[end] == '{':
				end = body_open(code, start, end)
		line_start = clean.rfind('\n', 0, start) + 1
		if clean[line_start:start].strip():
			line_start = start
		sig = dedent(clean[line_start:end].rstrip(' \t\n;'))
		if not sig:
			h.orphans.append(line)
			continue
		kind, name = classify(sig)
		stack = scopes.at(start)
		scope = '::'.join(n for k, n in stack if k == 'class')
		if any(k == 'namespace' and n == 'impl' for k, n in stack):
			scope = 'impl' + ('::' + scope if scope else '')
		entry = Entry(kind, name, sig + ('' if kind == 'macro' else ';'), body, scope, line)
		if kind == 'function':
			fn = function_of(sig)
			if fn:
				prefix = [n for k, n in stack if k in ('namespace', 'class')]
				entry.qual = qual_name('::'.join(prefix + [fn.name]))
				entry.params = param_key(fn.params)
		if kind in ('function', 'class', 'struct', 'union', 'enum') and end < len(code) and code[end] == '{':
			entry.sources.append((key, line_of(starts, start), line_of(starts, body_close(code, start, end))))
		h.entries.append(entry)
	return h


def md_text(text):
	parts = text.split('`')
	for i in range(0, len(parts), 2):
		parts[i] = HTML_LIKE.sub(r'\\\g<0>', parts[i])
	return '`'.join(parts)


def anchor(text):
	return re.sub(r'[^a-z0-9 -]', '', text.lower()).strip().replace(' ', '-')


def render_layer(layer, headers):
	out = [f'# {layer}', '', '_Generated by `tooling/autodoc.py` from `/* @DOC */` comments. Do not edit._', '']
	for h in headers:
		out += [f'## `{h.rel}`', '']
		if h.doc:
			out += [md_text(h.doc), '']
		for e in h.entries:
			label = f'{e.scope}::{e.name}' if e.scope else e.name
			out += [f'### {e.kind} `{label}`', '', '```c++', e.signature, '```', '']
			if e.text:
				out += [md_text(e.text), '']
		miss = h.missing()
		if miss:
			out += ['Undocumented: ' + ', '.join(f'`{n}`' for n in miss), '']
	return '\n'.join(out).rstrip() + '\n'


def render_index(layers):
	out = ['# ca reference', '', '_Generated by `tooling/autodoc.py`. Do not edit._', '',
		'| Layer | Headers | Documented entities |', '|---|---|---|']
	for layer, headers in layers:
		done, ents = coverage(headers)
		out.append(f'| [{layer}]({layer}.md) | {len(headers)} | {done}/{ents} |')
	for layer, headers in layers:
		out += ['', f'## [{layer}]({layer}.md)', '']
		for h in headers:
			summary = h.summary()
			link = f'{layer}.md#{anchor(h.rel)}'
			out.append(f'- [`{h.rel}`]({link})' + (f' - {md_text(summary)}' if summary else ''))
	return '\n'.join(out) + '\n'


HTML_CSS = '''
:root { --bg: #fdfdfc; --fg: #1f1f1d; --muted: #6c6b66; --border: #e4e2dc; --panel: #f4f3ef;
	--accent: #2f5fb3; --mono: ui-monospace, "JetBrains Mono", "SF Mono", Menlo, Consolas, monospace; color-scheme: light;
	--hl: #fbf1c7; --t-com: #7c7a72; --t-pp: #8a5a00; --t-str: #2f7a3e; --t-num: #a2421c; --t-kw: #7a3fb0;
	--t-ty: #1f6f8b; }
@media (prefers-color-scheme: dark) { :root:not([data-theme="light"]) { --bg: #151514; --fg: #e6e4de; --muted: #9a978f;
	--border: #2d2c29; --panel: #1d1c1a; --accent: #86a8ec; color-scheme: dark; --hl: #3a3420; --t-com: #85827a;
	--t-pp: #d6a85c; --t-str: #8fc58a; --t-num: #e59a77; --t-kw: #c3a2ef; --t-ty: #7cc4dc; } }
:root[data-theme="dark"] { --bg: #151514; --fg: #e6e4de; --muted: #9a978f; --border: #2d2c29; --panel: #1d1c1a;
	--accent: #86a8ec; color-scheme: dark; --hl: #3a3420; --t-com: #85827a; --t-pp: #d6a85c; --t-str: #8fc58a;
	--t-num: #e59a77; --t-kw: #c3a2ef; --t-ty: #7cc4dc; }
* { box-sizing: border-box; }
body { margin: 0; background: var(--bg); color: var(--fg); font: 15px/1.6 system-ui, -apple-system, "Segoe UI", sans-serif;
	display: grid; grid-template-columns: 260px minmax(0, 1fr); }
nav { position: sticky; top: 0; height: 100vh; overflow-y: auto; padding: 24px 16px; border-right: 1px solid var(--border);
	font-size: 13px; }
nav a { display: block; padding: 2px 6px; border-radius: 4px; color: var(--muted); text-decoration: none;
	overflow-wrap: anywhere; }
nav a:hover { color: var(--fg); background: var(--panel); }
nav .layer { display: inline; font-weight: 600; color: var(--fg); }
nav .current { color: var(--accent); }
nav ul { list-style: none; margin: 0 0 0 9px; padding: 0 0 0 8px; border-left: 1px solid var(--border); }
nav summary { cursor: pointer; padding: 2px 0; color: var(--muted); }
nav summary::marker { color: var(--muted); }
nav .layer-tree { margin-top: 8px; }
nav .layer-tree > summary { color: var(--fg); }
nav .home { font-size: 15px; font-weight: 700; color: var(--fg); margin-bottom: 8px; }
main { max-width: 920px; padding: 32px 40px 80px; }
a { color: var(--accent); }
h1 { font-size: 28px; margin: 0 0 4px; }
h2 { font-size: 20px; margin: 48px 0 12px; padding-top: 16px; border-top: 1px solid var(--border); }
h3 { font-size: 15px; margin: 28px 0 8px; }
h2 a.self, h3 a.self { color: inherit; text-decoration: none; }
.kind { font: 600 11px/1 system-ui, sans-serif; text-transform: uppercase; letter-spacing: .06em; color: var(--muted);
	margin-right: 8px; }
.note { color: var(--muted); font-size: 13px; margin: 0 0 24px; }
code { font-family: var(--mono); font-size: .9em; background: var(--panel); padding: 1px 4px; border-radius: 3px; }
pre { background: var(--panel); border: 1px solid var(--border); border-radius: 6px; padding: 10px 14px; overflow-x: auto;
	margin: 0 0 8px; }
pre code { background: none; padding: 0; font-size: 13px; line-height: 1.5; }
table { border-collapse: collapse; margin: 12px 0; font-size: 14px; display: block; overflow-x: auto; }
th, td { border: 1px solid var(--border); padding: 4px 10px; text-align: left; vertical-align: top; }
th { background: var(--panel); }
ul { padding-left: 22px; }
.entry p, .entry ul, .entry table { margin-top: 6px; }
.missing { color: var(--muted); font-size: 14px; }
.index li { margin: 2px 0; }
#search { width: 100%; margin: 0 0 12px; padding: 6px 10px; font: inherit; color: var(--fg); background: var(--panel);
	border: 1px solid var(--border); border-radius: 6px; }
#search:focus { outline: 2px solid var(--accent); outline-offset: -1px; }
nav .layer-link { display: block; padding: 2px 6px; }
#search-results { display: none; }
body.searching main > :not(#search-results) { display: none; }
body.searching #search-results { display: block; }
#search-results ul { list-style: none; padding: 0; margin: 0; }
#search-results a { display: grid; grid-template-columns: 76px minmax(0, 1fr); column-gap: 8px; padding: 6px 10px;
	border-radius: 6px; color: var(--fg); text-decoration: none; }
#search-results a:hover, #search-results a.active { background: var(--panel); }
#search-results a.active { box-shadow: inset 2px 0 0 var(--accent); }
#search-results .kind { grid-row: span 2; padding-top: 4px; }
#search-results code { background: none; padding: 0; overflow-wrap: anywhere; }
#search-results .ctx, #search-results .sum { grid-column: 2; color: var(--muted); font-size: 13px; }
#search-results .ctx { font-family: var(--mono); font-size: 12px; }
#search-results .sum:empty { display: none; }
.files { color: var(--muted); font-size: 13px; margin: 0 0 8px; }
details.source { margin: 6px 0 0; }
details.source > summary { cursor: pointer; color: var(--muted); font-size: 13px; width: max-content; }
details.source > summary span { font-family: var(--mono); font-size: 12px; }
details.source > summary:hover { color: var(--fg); }
.src-file { font: 12px var(--mono); margin: 10px 0 4px; }
.src-file a { color: var(--muted); }
pre.src { tab-size: 4; padding: 8px 0; }
pre.src .line { display: block; padding-right: 14px; }
pre.src .line:target { background: var(--hl); }
pre.src .ln { display: inline-block; width: 5ch; padding-right: 1.5ch; margin-right: 1ch; text-align: right;
	color: var(--muted); opacity: .6; text-decoration: none; user-select: none; border-right: 1px solid var(--border); }
pre.src .ln:hover { opacity: 1; }
.src-title { font-size: 20px; overflow-wrap: anywhere; }
.t-com { color: var(--t-com); font-style: italic; }
.t-pp, .t-attr { color: var(--t-pp); }
.t-str { color: var(--t-str); }
.t-num { color: var(--t-num); }
.t-kw { color: var(--t-kw); font-weight: 600; }
.t-ty { color: var(--t-ty); }
@media (max-width: 800px) {
	body { display: block; }
	nav { position: static; height: auto; border-right: none; border-bottom: 1px solid var(--border); padding: 16px; }
	nav .layer-tree > ul { display: none; }
	main { padding: 16px; }
}
'''
TABLE_RULE = re.compile(r'^\|[\s:|-]+\|?$')
KEYWORDS = ('alignas alignof asm break case catch class concept const consteval constexpr constinit const_cast '
	'continue co_await co_return co_yield decltype default delete do dynamic_cast else enum explicit export extern '
	'false for friend goto if inline mutable namespace new noexcept nullptr operator private protected public '
	'register reinterpret_cast requires return sizeof static static_assert static_cast struct switch template this '
	'thread_local throw true try typedef typeid typename union using virtual volatile while').split()
BUILTIN_TYPES = ('auto bool char char8_t char16_t char32_t double float int long short signed unsigned void '
	'wchar_t').split()
TOKEN = re.compile('|'.join([
	r'(?P<com>/\*.*?\*/|//[^\n]*)',
	r'(?P<pp>^[ \t]*#(?:[^\n\\]|\\.)*)',
	r'(?P<str>(?:u8|[uUL])?"(?:\\.|[^"\\\n])*"|(?<![\w\'])\'(?:\\.|[^\'\\\n])+\')',
	r'(?P<num>\b(?:0[xX][\da-fA-F\']+|0[bB][01\']+|\d[\d\']*(?:\.[\d\']*)?(?:[eE][+-]?\d+)?)[uUlLzZfF]*\b)',
	r'(?P<attr>\[\[.*?\]\])',
	r'(?P<kw>\b(?:' + '|'.join(KEYWORDS) + r')\b)',
	r'(?P<ty>\b(?:' + '|'.join(BUILTIN_TYPES) + r')\b)',
]), re.S | re.M)
SEARCH_JS = 'search.js'
SEARCH_SCRIPT = r'''
(() => {
	const input = document.getElementById('search');
	const main = document.querySelector('main');
	if (!input || !main) return;
	const panel = document.createElement('section');
	panel.id = 'search-results';
	main.prepend(panel);
	let hits = [], active = 0;
	const rank = (item, terms) => {
		const name = item[0].toLowerCase(), hay = name + ' ' + item[3].toLowerCase() + ' ' + item[1];
		if (!terms.every(t => hay.includes(t))) return -1;
		const q = terms[0], base = name.slice(Math.max(name.lastIndexOf(':'), name.lastIndexOf('/')) + 1);
		const kind = item[1] === 'file' ? 0.5 : 0;
		if (base === q) return kind;
		if (base.startsWith(q)) return 1 + kind + base.length / 1000;
		if (name.startsWith(q)) return 2 + kind;
		const i = name.indexOf(q);
		return i >= 0 ? 3 + kind + i / 1000 : 5 + kind;
	};
	const show = () => {
		panel.replaceChildren();
		const head = document.createElement('p');
		head.className = 'note';
		head.textContent = hits.length ? `${hits.length === 60 ? '60+' : hits.length} results` : 'No results';
		panel.append(head);
		const list = document.createElement('ul');
		hits.forEach((item, i) => {
			const li = document.createElement('li'), a = document.createElement('a');
			a.href = item[2];
			if (i === active) a.className = 'active';
			const kind = document.createElement('span'), name = document.createElement('code');
			const ctx = document.createElement('span'), sum = document.createElement('span');
			kind.className = 'kind'; kind.textContent = item[1];
			name.textContent = item[0];
			ctx.className = 'ctx'; ctx.textContent = item[3];
			sum.className = 'sum'; sum.textContent = item[4];
			a.append(kind, name, ctx, sum);
			li.append(a);
			list.append(li);
		});
		panel.append(list);
		const cur = list.querySelector('.active');
		if (cur) cur.scrollIntoView({block: 'nearest'});
	};
	const search = () => {
		const terms = input.value.trim().toLowerCase().split(/\s+/).filter(Boolean);
		document.body.classList.toggle('searching', terms.length > 0);
		if (!terms.length) return;
		hits = SEARCH_INDEX.map(item => [rank(item, terms), item]).filter(([r]) => r >= 0)
			.sort((a, b) => a[0] - b[0] || a[1][0].length - b[1][0].length).slice(0, 60).map(([, item]) => item);
		active = 0;
		show();
	};
	input.addEventListener('input', search);
	input.addEventListener('keydown', ev => {
		if (ev.key === 'ArrowDown' || ev.key === 'ArrowUp') {
			ev.preventDefault();
			active = (active + (ev.key === 'ArrowDown' ? 1 : hits.length - 1)) % Math.max(hits.length, 1);
			show();
		} else if (ev.key === 'Enter' && hits[active]) {
			location.href = hits[active][2];
			input.value = '';
			search();
		} else if (ev.key === 'Escape') {
			input.value = '';
			search();
			input.blur();
		}
	});
	panel.addEventListener('click', ev => {
		if (ev.target.closest('a')) {
			input.value = '';
			search();
		}
	});
	document.addEventListener('keydown', ev => {
		if (ev.key === '/' && document.activeElement !== input && !ev.ctrlKey && !ev.metaKey) {
			ev.preventDefault();
			input.focus();
			input.select();
		}
	});
	const open = () => {
		const el = location.hash && document.getElementById(decodeURIComponent(location.hash.slice(1)));
		const det = el && el.closest('details');
		if (det) det.open = true;
	};
	addEventListener('hashchange', open);
	open();
})();
'''


def esc(text):
	return html.escape(text, quote=True)


def html_inline(text):
	parts = text.split('`')
	if len(parts) % 2 == 0:
		parts[-2:] = ['`'.join(parts[-2:])]
	return ''.join(f'<code>{esc(p)}</code>' if i % 2 else esc(p) for i, p in enumerate(parts))


def table_cells(row):
	row = row.strip()
	row = row[1:-1] if row.endswith('|') and len(row) > 1 else row[1:]
	cells, cell, code = [], '', False
	for ch in row:
		if ch == '`':
			code = not code
		if ch == '|' and not code:
			cells.append(cell.strip())
			cell = ''
		else:
			cell += ch
	return cells + [cell.strip()]


def html_table(rows):
	out = ['<table>']
	if len(rows) > 1 and TABLE_RULE.match(rows[1]):
		out.append('<thead><tr>' + ''.join(f'<th>{html_inline(c)}</th>' for c in table_cells(rows[0])) + '</tr></thead>')
		rows = rows[2:]
	out.append('<tbody>')
	for r in rows:
		if not TABLE_RULE.match(r):
			out.append('<tr>' + ''.join(f'<td>{html_inline(c)}</td>' for c in table_cells(r)) + '</tr>')
	return ''.join(out) + '</tbody></table>'


def html_blocks(text):
	out = []
	for block in text.split('\n\n'):
		lines = block.split('\n')
		if not lines[0].startswith(('- ', '|')):
			out.append(f'<p>{html_inline(" ".join(lines))}</p>')
			continue
		group = []
		for l in lines + ['']:
			if group and not l.startswith('- ' if group[0].startswith('- ') else '|'):
				if group[0].startswith('- '):
					out.append('<ul>' + ''.join(f'<li>{html_inline(g[2:])}</li>' for g in group) + '</ul>')
				else:
					out.append(html_table(group))
				group = []
			if l:
				group.append(l)
	return '\n'.join(out)


def html_id(*parts):
	return '-'.join(re.sub(r'[^\w-]+', '-', p).strip('-') for p in parts)


def html_tree(headers, page):
	root = {}
	for h in headers:
		node = root
		for part in h.rel.split('/')[:-1]:
			node = node.setdefault(part + '/', {})
		node[h.rel.split('/')[-1]] = h
	def walk(node):
		files = [(k, v) for k, v in node.items() if not isinstance(v, dict)]
		dirs = [(k, v) for k, v in node.items() if isinstance(v, dict)]
		out = ['<ul>']
		out += [f'<li><a href="{page}#{html_id(h.rel)}">{esc(name)}</a></li>' for name, h in files]
		out += [f'<li><details open><summary>{esc(name)}</summary>{walk(sub)}</details></li>' for name, sub in dirs]
		return ''.join(out) + '</ul>'
	return walk(root)


def highlight(text):
	out = []
	pos = 0
	for m in TOKEN.finditer(text):
		if m.start() > pos:
			out.append(('', text[pos:m.start()]))
		out.append((m.lastgroup, m.group()))
		pos = m.end()
	out.append(('', text[pos:]))
	lines, cur = [], []
	for cls, tok in out:
		for i, part in enumerate(tok.split('\n')):
			if i:
				lines.append(''.join(cur))
				cur = []
			if part:
				cur.append(f'<span class="t-{cls}">{esc(part)}</span>' if cls else esc(part))
	return lines + [''.join(cur)]


def source_page(key):
	return key.replace('/', '.') + '.html'


def html_source(text, first, link):
	lines = text.split('\n')
	body = [l for l in lines if l.strip()]
	pad = min((len(l) - len(l.lstrip('\t ')) for l in body), default=0)
	lines = [l[pad:] if l[:pad].strip('\t ') == '' else l.lstrip('\t ') for l in lines]
	out = []
	for n, line in enumerate(highlight('\n'.join(lines)), first):
		out.append(f'<span class="line" id="L{n}"><a class="ln" href="{link}#L{n}">{n}</a>{line}</span>'
			if link == '' else f'<span class="line"><a class="ln" href="{link}#L{n}">{n}</a>{line}</span>')
	return '<pre class="src"><code>' + ''.join(out) + '</code></pre>'


def html_slice(sources, key, first, last):
	text = '\n'.join(sources[key].split('\n')[first - 1:last])
	page = source_page(key)
	return (f'<div class="src-file"><a href="{page}#L{first}">{esc(key)}:{first}</a></div>'
		+ html_source(text, first, page))


def html_files(h):
	return ('<p class="files">Source: ' + ' &middot; '.join(
		f'<a href="{source_page(k)}"><code>{esc(k.rsplit("/", 1)[-1])}</code></a>' for k in h.files) + '</p>')


def html_page(title, current, layers, body, tree=True):
	nav = ['<a class="home" href="index.html">ca reference</a>',
		'<input id="search" type="search" placeholder="Search (press /)" autocomplete="off" '
		'spellcheck="false" aria-label="Search the reference">']
	for layer, headers in layers:
		here = layer == current
		page = '' if here else f'{esc(layer)}.html'
		if not tree:
			nav.append(f'<a class="layer layer-link" href="{esc(layer)}.html">{esc(layer)}</a>')
			continue
		nav.append(f'<details class="layer-tree"{" open" if here else ""}><summary>'
			f'<a class="layer{" current" if here else ""}" href="{esc(layer)}.html">{esc(layer)}</a></summary>'
			f'{html_tree(headers, page)}</details>')
	return ('<!doctype html>\n<html lang="en">\n<head>\n<meta charset="utf-8">\n'
		'<meta name="viewport" content="width=device-width, initial-scale=1">\n'
		f'<title>{esc(title)} - ca reference</title>\n<style>{HTML_CSS}</style>\n'
		f'<script src="{SEARCH_JS}" defer></script>\n</head>\n<body>\n'
		f'<nav>{"".join(nav)}</nav>\n<main>\n{body}\n</main>\n</body>\n</html>\n')


def html_entries(h):
	seen = {}
	for e in h.entries:
		label = f'{e.scope}::{e.name}' if e.scope else e.name
		eid = html_id(h.rel, label)
		seen[eid] = seen.get(eid, 0) + 1
		if seen[eid] > 1:
			eid += f'-{seen[eid]}'
		yield e, label, eid


def render_layer_html(layer, headers, layers, sources):
	out = [f'<h1>{esc(layer)}</h1>',
		'<p class="note">Generated by <code>tooling/autodoc.py</code> from <code>/* @DOC */</code> comments.</p>']
	for h in headers:
		hid = html_id(h.rel)
		out.append(f'<section id="{hid}"><h2><a class="self" href="#{hid}"><code>{esc(h.rel)}</code></a></h2>')
		out.append(html_files(h))
		if h.doc:
			out.append(html_blocks(h.doc))
		for e, label, eid in html_entries(h):
			src = ''
			if e.sources:
				where = sorted({k.rsplit('/', 1)[-1] for k, _, _ in e.sources}, key=lambda n: not n.endswith('.hpp'))
				src = (f'<details class="source"><summary>source <span>{esc(", ".join(where))}</span></summary>'
					+ ''.join(html_slice(sources, *s) for s in e.sources) + '</details>')
			out.append(f'<div class="entry" id="{eid}"><h3><a class="self" href="#{eid}">'
				f'<span class="kind">{esc(e.kind)}</span><code>{esc(label)}</code></a></h3>'
				f'<pre><code>{esc(e.signature)}</code></pre>' + (html_blocks(e.text) if e.text else '') + src + '</div>')
		miss = h.missing()
		if miss:
			out.append('<p class="missing">Undocumented: ' + ', '.join(f'<code>{esc(n)}</code>' for n in miss) + '</p>')
		out.append('</section>')
	return html_page(layer, layer, layers, '\n'.join(out))


def render_index_html(layers):
	out = ['<h1>ca reference</h1>', '<p class="note">Generated by <code>tooling/autodoc.py</code>.</p>',
		'<table><thead><tr><th>Layer</th><th>Headers</th><th>Documented entities</th></tr></thead><tbody>']
	for layer, headers in layers:
		done, ents = coverage(headers)
		out.append(f'<tr><td><a href="{esc(layer)}.html">{esc(layer)}</a></td><td>{len(headers)}</td>'
			f'<td>{done}/{ents}</td></tr>')
	out.append('</tbody></table>')
	for layer, headers in layers:
		out.append(f'<h2><a href="{esc(layer)}.html">{esc(layer)}</a></h2><ul class="index">')
		for h in headers:
			summary = h.summary()
			out.append(f'<li><a href="{esc(layer)}.html#{html_id(h.rel)}"><code>{esc(h.rel)}</code></a>'
				+ (f' - {html_inline(summary)}' if summary else '') + '</li>')
		out.append('</ul>')
	return html_page('Index', None, layers, '\n'.join(out))


def render_md(layers, sources):
	pages = {layer + '.md': render_layer(layer, headers) for layer, headers in layers}
	pages['index.md'] = render_index(layers)
	return pages


def render_source_html(key, text, layers, docs):
	h = docs.get(key[:-len('.cpp')] + '.hpp')
	links = []
	if h:
		links.append(f'<a href="{esc(key.split("/", 1)[0])}.html#{html_id(h.rel)}">documentation</a>')
		links += [f'<a href="{source_page(k)}"><code>{esc(k.rsplit("/", 1)[-1])}</code></a>' for k in h.files if k != key]
	body = (f'<h1 class="src-title"><code>{esc(key)}</code></h1>'
		+ (f'<p class="files">{" &middot; ".join(links)}</p>' if links else '')
		+ html_source(text.rstrip('\n'), 1, ''))
	return html_page(key, None, layers, body, tree=False)


def summary_of(text):
	first = re.split(r'(?<=[.!?])\s', text.split('\n')[0], maxsplit=1)[0]
	return first if len(first) <= 100 else first[:97].rstrip() + '...'


def search_index(layers, sources):
	items = []
	for layer, headers in layers:
		for h in headers:
			items.append([h.rel, 'header', f'{layer}.html#{html_id(h.rel)}', layer, h.summary()])
			items += [[label, e.kind, f'{layer}.html#{eid}', f'{layer}/{h.rel}', summary_of(e.text)]
				for e, label, eid in html_entries(h)]
	items += [[key.rsplit('/', 1)[-1], 'file', source_page(key), key, ''] for key in sources]
	return items


def render_html(layers, sources):
	pages = {layer + '.html': render_layer_html(layer, headers, layers, sources) for layer, headers in layers}
	pages['index.html'] = render_index_html(layers)
	docs = {h.key: h for _, headers in layers for h in headers}
	shown = {layer for layer, _ in layers}
	for key, text in sources.items():
		if key.split('/', 1)[0] in shown:
			pages[source_page(key)] = render_source_html(key, text, layers, docs)
	index = json.dumps(search_index(layers, sources), separators=(',', ':'))
	pages[SEARCH_JS] = f'const SEARCH_INDEX = {index};\n{SEARCH_SCRIPT}'
	return pages


RENDERERS = {'md': render_md, 'html': render_html}


def coverage(headers):
	ents = sum(len(h.entities) for h in headers)
	done = sum(len(h.entities & h.documented()) for h in headers)
	return done, ents


def write_if_changed(path, content):
	try:
		with open(path, encoding='utf-8') as fh:
			if fh.read() == content:
				return
	except FileNotFoundError:
		pass
	with open(path, 'w', encoding='utf-8') as fh:
		fh.write(content)


def owned(name, fmt):
	if fmt == 'html' and name == SEARCH_JS:
		return True
	return (LAYER.match(name) or name.startswith(FACADE + '.') or name.startswith('index.')) and name.endswith('.' + fmt)


def default_out(fmt):
	return os.path.join(REPO, 'doc', *(['html'] if fmt == 'html' else []))


def fclean_targets(opts):
	formats = [opts.format] if opts.format else sorted(RENDERERS)
	return [(opts.out or default_out(fmt), fmt) for fmt in formats]


def main(argv=None):
	ap = argparse.ArgumentParser(description='Generate Markdown or HTML docs from /* @DOC */ comments.')
	ap.add_argument('lib', nargs='?', default=os.path.join(REPO, 'src', 'lib'))
	ap.add_argument('-o', '--out', help='output directory (default: doc for md, doc/html for html)')
	ap.add_argument('-f', '--format', choices=sorted(RENDERERS), help='default md')
	ap.add_argument('--check', action='store_true', help='exit 1 on orphan @DOC or undocumented entities')
	ap.add_argument('--missing', action='store_true', help='print undocumented entities')
	ap.add_argument('--bench', nargs='*', metavar='NAME', help='run the benchmarks quoted by --BENCH blocks (all, or NAME...) and rewrite them')
	ap.add_argument('--cxx', default='clang++,g++', help='compilers for --bench, comma-separated (default clang++,g++)')
	cleanup.add_arguments(ap)
	opts = ap.parse_args(argv)
	if opts.clean or opts.fclean:
		targets = fclean_targets(opts)
		pages = lambda: [os.path.join(d, n) for d, fmt in targets if os.path.isdir(d) for n in sorted(os.listdir(d)) if owned(n, fmt)]
		return cleanup.run(opts, 'autodoc', cleanup.bytecode('norm', 'bench_tables'), pages, [d for d, _ in targets])
	if opts.bench is not None:
		try:
			bench_tables.run(REPO, opts.lib, opts.bench, [c for c in opts.cxx.split(',') if c])
		except (RuntimeError, subprocess.CalledProcessError) as e:
			print(f'autodoc: benchmark failed: {e}', file=sys.stderr)
			return 1
	opts.format = opts.format or 'md'
	if opts.out is None:
		opts.out = default_out(opts.format)

	layers, sources, defs = [], {}, {}
	bench_errors = 0
	for layer in sorted(d for d in os.listdir(opts.lib) if LAYER.match(d) or d == FACADE):
		root = os.path.join(opts.lib, layer)
		headers = []
		for d, dirs, names in os.walk(root):
			dirs.sort()
			for n in sorted(names):
				if n.endswith(('.hpp', '.cpp')):
					path = os.path.join(d, n)
					key = os.path.relpath(path, opts.lib).replace(os.sep, '/')
					with open(path, encoding='utf-8') as fh:
						sources[key] = fh.read()
					if opts.check:
						markers = bench_tables.check(path, sources[key], REPO)
						for message in markers:
							print(message, file=sys.stderr)
						bench_errors += len(markers)
					definitions(key, sources[key], defs)
					if n.endswith('.hpp'):
						headers.append(parse(path, os.path.relpath(path, root), key))
		for h in headers:
			impl = h.key[:-len('.hpp')] + '.cpp'
			if impl in sources:
				h.files.append(impl)
		layers.append((layer, headers))
	resolve_sources(layers, defs)

	os.makedirs(opts.out, exist_ok=True)
	pages = RENDERERS[opts.format]([(layer, headers) for layer, headers in layers if headers], sources)
	for name, content in pages.items():
		write_if_changed(os.path.join(opts.out, name), content)
	for name in os.listdir(opts.out):
		if owned(name, opts.format) and name not in pages:
			os.remove(os.path.join(opts.out, name))

	bad = bench_errors
	for layer, headers in layers:
		for h in headers:
			for line in h.orphans:
				print(f'{os.path.join(opts.lib, layer, h.rel)}:{line}: @DOC not followed by a declaration', file=sys.stderr)
				bad += 1
			if opts.missing or opts.check:
				for name in h.missing():
					print(f'{os.path.join(opts.lib, layer, h.rel)}: undocumented `{name}`', file=sys.stderr)
					bad += 1
	headers = [h for _, hs in layers for h in hs]
	done, ents = coverage(headers)
	print(f'autodoc: {done}/{ents} entities documented, {len(headers)} headers -> {opts.out}', file=sys.stderr)
	return 1 if bad and opts.check else 0


if __name__ == '__main__':
	sys.exit(main())
