#!/usr/bin/env python3
"""Enforce the ca coding style (STYLE.md) on C++ sources.

Usage: norm.py [paths...]   (default: <repo>/src/lib, else <repo>/lib)

Heuristic, line/scope based; no full C++ parse. Exit 1 on errors
(or warnings with --werror), 2 on unreadable or missing inputs.
`--list-rules` prints every rule.

Spaced-capital banners (`// -- C A  N A M E ---`) are squashed in place
(`// -- CA NAME ---`), dashes re-padded to column 78; each rewrite is printed.
`--no-fix` reports them instead of rewriting.

Per-line suppression, as a comment on the line a diagnostic is reported on:
	// norm: ignore                 all rules
	// <rule>: disable              one rule (ignore / disable interchangeable)
	// rule1, rule2, rule3: ignore  several rules
A selector names a rule or a family: `guard` selects guard-open, guard-close, ...
(also for --only / --disable).

Design: each file is parsed once into an ordered stream of events (File, Line,
Comment, Decl, Close, ...). A rule is one `rule(name, severity, Event, message,
check)` entry; `check(event, ctx)` returns None, one `at(li, col, **fields)` or
an iterable of them, and the engine reports `message.format(**fields)` there,
in event order, as found.
"""

import argparse
import bisect
import itertools
import os
import re
import shutil
import sys
import tempfile
from collections import defaultdict
from collections.abc import Callable, Iterable
from dataclasses import dataclass, field
from functools import cached_property, lru_cache
from typing import Any

# -- configuration -----------------------------------------------------------

NAMESPACE = 'ca'
BANNER_WIDTH = 78
SOURCE_EXT = ('.hpp', '.cpp')
LIB_DIRS = (os.path.join('src', 'lib'), 'lib')
EXCLUDE_DIRS = ('build',)
DEFAULT_SYSTEM_LAYER = 3
FACADE = 'ca'
FACADE_LAYER = 1 << 30
SUPPRESS_ALL = 'norm'

FREESTANDING = {
	'stddef.h', 'stdint.h', 'stdarg.h', 'stdbool.h', 'stdalign.h',
	'stdnoreturn.h', 'limits.h', 'float.h', 'iso646.h',
}

BUILTIN_NAMES = {
	'i8', 'u8', 'i16', 'u16', 'i32', 'u32', 'i64', 'u64', 'usz', 'isz',
	'sint', 'uint', 'f32', 'f64', 'byte',
	'move', 'forward', 'exchange', 'declval', 'launder', 'launder_cast',
}

SECTIONS = [
	('friends',),
	('private types',),
	('public types',),
	('private members',),
	('public lifecycle',),
	('assignment operators',),
	('iterators',),
	('accessors',),
	('element access',),
	('modifiers', 'methods'),
	('private methods',),
	('assertions',),
]
SECTION_RANK = {name: rank for rank, names in enumerate(SECTIONS) for name in names}

# -- patterns ----------------------------------------------------------------

BODY_KINDS = ('function', 'block', 'switch')
SPEC_WORDS = {'static', 'constexpr', 'consteval', 'inline', 'virtual', 'explicit', 'friend', 'extern', 'constinit', 'template'}
NOT_FUNC = {'decltype', 'sizeof', 'alignof', 'alignas', 'noexcept', 'static_assert', 'requires', 'typeid', '__attribute__'}
FLAG_AFTER = {
	'return', 'const', 'constexpr', 'consteval', 'constinit', 'static', 'typename', 'inline', 'volatile',
	'case', 'mutable', 'thread_local', 'extern', 'friend', 'co_return', 'co_yield', 'sizeof', 'alignof',
	'decltype', 'noexcept', 'new', 'delete', 'else', 'do', 'requires', 'not', 'and', 'or', 'explicit', 'virtual',
}
DECL_START = re.compile(r'(?<![\w:])(auto|class|struct|union|constexpr|consteval|static|inline|explicit|friend|virtual|concept|using)\b|\[\[')

IDENT = re.compile(r'[A-Za-z_]\w*')
SNAKE = re.compile(r'_?[a-z][a-z0-9_]*')
UPPER = re.compile(r'[A-Z][A-Z0-9_]*')
SPACED_CAPS = re.compile(r'[A-Z0-9_](?: [A-Z0-9_])*(?:  [A-Z0-9_](?: [A-Z0-9_])*)*')
CAPS = re.compile(r'[A-Z0-9_]+(?: [A-Z0-9_]+)*')
BANNER = re.compile(r'// -- (\S.*?) -+')
BANNER_LINE = re.compile(r'^([ \t]*)// -- (\S.*?) -+[ \t]*$')
LOWER_TITLE = re.compile(r'[a-z0-9]+(?: [a-z0-9]+)*')
CLASS_HEAD = re.compile(
	r'(class|struct|union)\b\s*(?:(?:\[\[.*?\]\]|alignas\s*\(.*?\))\s*)*'
	r'(?!final\b)([A-Za-z_][\w:]*)?(\s*<.*?>)?\s*(final)?\s*(?::(?!:).*)?$')
LAMBDA = re.compile(
	r'\](?:\s*\((?:[^()]|\([^()]*\))*\))?\s*'
	r'(?:(?:mutable|constexpr|consteval|static|noexcept(?:\s*\([^()]*\))?)\s*)*'
	r'(?:->\s*[^{};=]+)?$')
ACCESS = re.compile(r'(public|private|protected)\s*:(?!:)')
ALIAS = re.compile(r'^\t*using\s+\w+(\s*)=(?!=)')
ASSIGN = re.compile(r'^\t* *[A-Za-z_]\w*(?:(?:\.|->)\w+)*(?:\[[^\]]*\])?(\s*)=(?!=)')
LITERAL = re.compile(r"(?<![\w.])(?:0[xX][0-9a-fA-F']+|0[bB][01']+|\d[\d']*)([uUlLzZ]+)\b")
SUPPRESSION = re.compile(r'//\s*([\w-]+(?:\s*,\s*[\w-]+)*)\s*:\s*(?:ignore|disable)\s*')
INIT_SAME_LINE = re.compile(r'\)\s*(?:noexcept\s*(?:\([^()]*\))?\s*)?:(?!:)\s*[A-Za-z_][\w:]*(?:<[^>]*>)?\s*[{(]')
LAYER_DIR = re.compile(r'(\d+)-([^/\\]+)$')
INCLUDE = re.compile(r'\s*#\s*include\s*([<"])([^>"]*)[>"]')
TEMPLATE_HEAD = re.compile(r'(?<![\w.:>])template\s*<')

# -- lexing ------------------------------------------------------------------


def lex(text):
	"""Blank out comments (spaces) and literal contents (underscores), keeping offsets."""
	code = list(text)
	comments = []
	i, n = 0, len(text)

	def blank(a, b, fill):
		for k in range(a, b):
			if code[k] != '\n':
				code[k] = fill

	while i < n:
		c = text[i]
		if c == '/' and text.startswith('//', i):
			j = text.find('\n', i)
			j = n if j < 0 else j
			comments.append((i, text[i:j], False))
			blank(i, j, ' ')
			i = j
		elif c == '/' and text.startswith('/*', i):
			j = text.find('*/', i + 2)
			j = n if j < 0 else j + 2
			comments.append((i, text[i:j], True))
			blank(i, j, ' ')
			i = j
		elif c in '"\'':
			k = i
			while k > 0 and (text[k - 1].isalnum() or text[k - 1] == '_'):
				k -= 1
			prefix = text[k:i]
			if c == "'" and prefix[:1].isdigit():
				i += 1
				continue
			if c == '"' and prefix.endswith('R'):
				open_ = text.find('(', i)
				if open_ >= 0:
					delim = text[i + 1:open_]
					j = text.find(')' + delim + '"', open_)
					j = n if j < 0 else j + len(delim) + 2
					blank(i + 1, j - 1, '_')
					i = j
					continue
			j = i + 1
			while j < n and text[j] != c and text[j] != '\n':
				j += 2 if text[j] == '\\' else 1
			blank(i + 1, min(j, n), '_')
			i = j + 1
		else:
			i += 1
	return ''.join(code), comments


class Source:
	"""A file split into raw lines, lexed code lines, comments and preprocessor flags."""

	def __init__(self, path, text):
		self.path = path
		self.text = text
		self.ext = os.path.splitext(path)[1]
		code, comments = lex(text)
		self.raw = text.split('\n')
		self.code = code.split('\n')
		starts = [0]
		for line in self.raw:
			starts.append(starts[-1] + len(line) + 1)
		self.comments = []
		for off, body, block in comments:
			li = bisect.bisect_right(starts, off) - 1
			self.comments.append((li, off - starts[li], body, block))
		self.pp = [False] * len(self.code)
		cont = False
		for li, line in enumerate(self.code):
			if cont or line.lstrip().startswith('#'):
				self.pp[li] = True
				cont = line.rstrip().endswith('\\')


# -- text helpers ------------------------------------------------------------


def tabs(s):
	return len(s) - len(s.lstrip('\t'))


def visual(s, tab):
	col = 0
	for ch in s:
		col = (col // tab + 1) * tab if ch == '\t' else col + 1
	return col


def match_close(t, i, o, c):
	depth = 0
	for k in range(i, len(t)):
		if t[k] == o:
			depth += 1
		elif t[k] == c:
			depth -= 1
			if depth == 0:
				return k
	return -1


def strip_templates(t):
	t = t.strip()
	headers = []
	while True:
		m = re.match(r'template\s*<', t)
		if not m:
			break
		k = match_close(t, m.end() - 1, '<', '>')
		if k < 0:
			break
		headers.append(t[:k + 1])
		t = t[k + 1:].strip()
	if t.startswith('requires'):
		depth = 0
		for k in range(len('requires'), len(t)):
			ch = t[k]
			if ch in '(<[':
				depth += 1
			elif ch in ')>]':
				depth -= 1
			elif depth <= 0 and DECL_START.match(t, k):
				t = t[k:]
				break
	return headers, t


def strip_attrs(t):
	t = t.strip()
	while True:
		if t.startswith('[['):
			j = t.find(']]')
			if j < 0:
				break
			t = t[j + 2:].strip()
		elif re.match(r'alignas\s*\(', t):
			j = match_close(t, t.find('('), '(', ')')
			if j < 0:
				break
			t = t[j + 1:].strip()
		elif (m := re.match(r'extern\s*"[^"]*"', t)):
			t = t[m.end():].strip()
		else:
			break
	return t


def prefix_only(t):
	headers, rest = strip_templates(t)
	return bool(t) and strip_attrs(rest) == '', bool(headers)


def name_start(head):
	i = len(head)
	while i > 0:
		ch = head[i - 1]
		if ch.isalnum() or ch in '_~:':
			i -= 1
		elif ch == '>':
			depth = 0
			j = i - 1
			while j >= 0:
				if head[j] == '>':
					depth += 1
				elif head[j] == '<':
					depth -= 1
					if depth == 0:
						break
				j -= 1
			if j < 0:
				break
			i = j
		else:
			break
	return i


def top_level(text):
	out, depth = [], 0
	for ch in text:
		if ch in '({[':
			depth += 1
		out.append(ch if depth <= (1 if ch in '({[' else 0) else ' ')
		if ch in ')}]':
			depth -= 1
	return ''.join(out)


def banner_title(body):
	m = BANNER.fullmatch(body)
	return m.group(1) if m else None


def is_spaced(title):
	return ' ' in title and SPACED_CAPS.fullmatch(title) is not None


@dataclass
class Func:
	specs: str
	name: str
	params: str
	rest: str


def split_function(t):
	ex = re.search(r'\bexplicit\s*\(', t)
	if ex:
		close = match_close(t, ex.end() - 1, '(', ')')
		if close >= 0:
			t = t[:ex.start()] + 'explicit ' + t[close + 1:].lstrip()
	m = re.search(r'\boperator\s*\(\s*\)\s*\(', t)
	open_ = m.end() - 1 if m else t.find('(')
	if open_ < 0 or re.match(r'\s*[*&]', t[open_ + 1:]):
		return None
	close = match_close(t, open_, '(', ')')
	if close < 0:
		return None
	head = t[:open_].rstrip()
	op = re.search(r'\boperator\b', head)
	if op:
		start = op.start()
		q = re.search(r'((?:~?\w+(?:<[^()]*?>)?\s*::\s*)*)$', head[:start])
		start -= len(q.group(1)) if q else 0
	else:
		start = name_start(head)
	name = head[start:].strip()
	specs = head[:start].strip()
	if not name or '=' in specs or '(' in specs or '{' in specs:
		return None
	base = name.split('::')[-1].strip()
	if base in NOT_FUNC or not re.match(r'~?[A-Za-z_]', base):
		return None
	return Func(specs, name, t[open_ + 1:close].strip(), t[close + 1:].strip())


def init_colon(rest):
	open_ = rest.find('(')
	close = match_close(rest, open_, '(', ')') if open_ >= 0 else -1
	return close >= 0 and re.search(r'(?<!:):(?!:)', rest[close + 1:]) is not None


# -- project layout ----------------------------------------------------------


@lru_cache(maxsize=None)
def system_layer(root):
	if root and os.path.isdir(root):
		for entry in sorted(os.listdir(root)):
			m = re.match(r'(\d+)-system$', entry)
			if m:
				return int(m.group(1))
	return DEFAULT_SYSTEM_LAYER


def find_lib_root(path):
	d = os.path.dirname(os.path.abspath(path))
	while d != os.path.dirname(d):
		if os.path.basename(d) == 'lib':
			return d
		d = os.path.dirname(d)
	return None


@dataclass(frozen=True)
class Layout:
	"""Where a file sits in the layered tree: lib root, path under it, layer."""
	root: str | None = None
	rel: str | None = None
	layer: int | None = None
	layer_name: str | None = None
	system_layer: int = DEFAULT_SYSTEM_LAYER

	@classmethod
	def of(cls, path, root=None):
		root = root or find_lib_root(path)
		if not root:
			return cls()
		rel = os.path.relpath(os.path.abspath(path), root)
		top = rel.split(os.sep)[0]
		layer = layer_name = None
		m = LAYER_DIR.match(top)
		if m:
			layer, layer_name = int(m.group(1)), m.group(2)
		elif top == FACADE and top != rel:
			layer, layer_name = FACADE_LAYER, 'include'
		return cls(root, rel, layer, layer_name, system_layer(root))

	@property
	def inside(self):
		return self.rel is not None and not self.rel.startswith('..')

	def guard(self):
		if not self.rel or self.layer_name is None:
			return None
		parts = self.rel.split(os.sep)
		parts[0] = self.layer_name
		return '_'.join([NAMESPACE] + [re.sub(r'[^a-z0-9]+', '_', p.lower()) for p in parts])


# -- events ------------------------------------------------------------------


@dataclass(frozen=True)
class At:
	"""Snapshot of the enclosing scope when an event was seen."""
	kind: str
	name: str
	keyword: str
	indent: int
	inner: int
	in_body: bool
	access: str
	ns: tuple
	cls: str | None
	sections: tuple

	@property
	def file_scope(self):
		return self.kind in ('file', 'linkage')

	@property
	def top_ca(self):
		return self.kind == 'namespace' and self.ns == (NAMESPACE,)


@dataclass
class File:
	"""Once per file, first."""


@dataclass
class Suppression:
	li: int
	col: int
	names: tuple


@dataclass
class Directive:
	li: int
	keyword: str
	arg: str


@dataclass
class Include:
	li: int
	quoted: bool
	path: str
	layer: int | None
	prev_layer: int
	after_system: bool


@dataclass
class Guard:
	"""Once per header, after the last line: every directive in order."""
	directives: tuple
	expected: str | None

	@property
	def opening(self):
		d = self.directives
		return len(d) >= 2 and d[0][1] == 'ifndef' and d[1][1] == 'define'

	@property
	def name(self):
		return self.directives[0][2]


@dataclass
class Line:
	"""A non-blank, non-preprocessor line, before its characters are walked."""
	li: int
	code: str
	raw: str
	at: At
	cont: bool
	has_template: bool
	stmt_indent: int | None

	@cached_property
	def stripped(self):
		return self.code.lstrip()

	@cached_property
	def tabs(self):
		return tabs(self.raw)

	@cached_property
	def templates(self):
		heads, covered = [], -1
		for m in TEMPLATE_HEAD.finditer(self.code):
			if m.start() < covered:
				continue
			end = match_close(self.code, m.end() - 1, '<', '>')
			heads.append((m.start(), end))
			if end < 0:
				break
			covered = end
		return heads


@dataclass
class CommentLine:
	"""A line holding only comments."""
	li: int
	tabs: int
	at: At
	cont: bool


@dataclass
class Comment:
	li: int
	col: int
	body: str
	block: bool
	at: At
	lead: str

	@cached_property
	def title(self):
		return banner_title(self.body)

	@property
	def is_banner(self):
		return self.body.startswith('// --') or self.body.startswith('//--')


@dataclass
class Decl:
	"""A declaration outside function bodies.

	what: namespace, class/struct/union/enum (head or forward), alias, using,
	concept, function, variable, other (still carries template headers).
	"""
	li: int
	at: At
	what: str
	name: str = ''
	full: str = ''
	headers: tuple = ()
	definition: bool = False
	head: bool = False
	final: bool | None = None
	fn: Func | None = None
	parts: tuple = ()
	words: tuple = ()
	nodiscard: bool = False
	is_ctor: bool = False
	is_dtor: bool = False
	is_op: bool = False
	is_conv: bool = False

	@property
	def special(self):
		return self.is_ctor or self.is_dtor or self.is_op

	@property
	def entity(self):
		if not self.at.top_ca or not self.name or (self.what == 'function' and self.is_op):
			return None
		return self.name if self.what not in ('namespace', 'using', 'other') else None


@dataclass
class Close:
	li: int
	col: int
	kind: str
	name: str
	keyword: str
	in_body: bool
	has_self: bool
	open_li: int
	rest: str


@dataclass
class Body:
	"""A closed function body (not a lambda)."""
	li: int
	col: int
	open_li: int
	open_col: int
	statements: int
	inner_empty: bool
	first_blank: bool

	@property
	def inline(self):
		return self.open_li == self.li

	@property
	def first(self):
		return self.open_li + 1


@dataclass
class Brace:
	li: int
	col: int
	kind: str
	unmatched: bool


@dataclass
class AlignGroup:
	"""Consecutive alias or assignment lines of one scope: (li, kind, lead, space)."""
	items: list


@dataclass
class Entities:
	"""Once per file, last: names declared directly in namespace ca."""
	stem: str
	header: bool
	names: set


# -- rule engine -------------------------------------------------------------

ERROR, WARNING = 'error', 'warning'


@dataclass(frozen=True)
class Hit:
	li: int
	col: int | None = None
	fields: dict = field(default_factory=dict)


def at(li, col=None, **fields):
	"""Position of a finding (0-based line; col None = first non-blank); fields fill the message."""
	return Hit(li, col, fields)


@dataclass(frozen=True)
class Rule:
	name: str
	severity: str
	on: type
	message: str
	check: Callable[[Any, Any], Hit | Iterable[Hit] | None]

	def selected_by(self, selector):
		return self.name == selector or self.name.startswith(selector + '-')

	def hits(self, event, ctx):
		found = self.check(event, ctx)
		if found is None:
			return ()
		return (found,) if isinstance(found, Hit) else found


RULES = {}


def rule(name, severity, on, message, check=None):
	"""Register a rule; without check, decorates the check function."""
	def register(fn):
		assert name not in RULES, name
		RULES[name] = Rule(name, severity, on, message, fn)
		return fn
	return register(check) if check else register


def select(selector):
	return [r for r in RULES.values() if r.selected_by(selector)]


@dataclass
class Ctx:
	src: Source
	layout: Layout
	opts: Any
	index: set


def banned(pattern):
	regex = re.compile(pattern)
	return lambda l, c: [at(l.li, m.start(), token=m.group()) for m in regex.finditer(l.code)]


# -- rules: file -------------------------------------------------------------

rule('crlf', ERROR, File, 'CR characters in file',
	lambda f, c: at(0, 0) if '\r' in c.src.text else None)
rule('eof-newline', ERROR, File, 'missing newline at end of file',
	lambda f, c: at(len(c.src.raw) - 1, 0) if c.src.text and not c.src.text.endswith('\n') else None)
rule('layer-dir', ERROR, File, '{rel} not inside a lib/<n>-<layer>/ directory',
	lambda f, c: at(0, 0, rel=c.layout.rel) if c.layout.inside and c.layout.layer is None else None)
rule('directive', ERROR, Suppression, 'unknown rule `{name}` in suppression',
	lambda s, c: [at(s.li, s.col, name=n) for n in s.names if n != SUPPRESS_ALL and not select(n)])

# -- rules: preprocessor -----------------------------------------------------

rule('pragma-once', ERROR, Directive, 'use an include guard, not #pragma once',
	lambda d, c: at(d.li, 0) if d.keyword == 'pragma' and d.arg.startswith('once') else None)


@rule('macro-name', ERROR, Directive, 'macro `{name}` must be lowercase, prefixed ca_')
def _(d, c):
	m = IDENT.match(d.arg) if d.keyword == 'define' else None
	if m and not re.fullmatch(r'ca_[a-z0-9_]*', m.group()):
		return at(d.li, None, name=m.group())


rule('include-order-system', ERROR, Include, 'project include after system header',
	lambda i, c: at(i.li, 0) if i.quoted and i.after_system else None)
rule('include-root', ERROR, Include, '"{path}" not rooted at lib/<n>-<layer>/',
	lambda i, c: at(i.li, 0, path=i.path) if i.quoted and i.layer is None else None)
rule('layering', ERROR, Include, 'layer {own} includes higher layer {layer}: "{path}"',
	lambda i, c: at(i.li, 0, own=c.layout.layer, layer=i.layer, path=i.path)
	if i.layer is not None and c.layout.layer is not None and i.layer > c.layout.layer else None)
rule('include-order-layer', ERROR, Include, 'layer {layer} include after layer {prev}',
	lambda i, c: at(i.li, 0, layer=i.layer, prev=i.prev_layer)
	if i.layer is not None and i.layer < i.prev_layer else None)
rule('include-missing', WARNING, Include, '"{path}" not found under {root}',
	lambda i, c: at(i.li, 0, path=i.path, root=c.layout.root)
	if i.layer is not None and c.layout.root and not os.path.exists(os.path.join(c.layout.root, i.path)) else None)
rule('cxx-header', ERROR, Include, 'C++ standard header <{path}>',
	lambda i, c: at(i.li, 0, path=i.path) if not i.quoted and not i.path.endswith('.h') else None)
rule('system-include', ERROR, Include, '<{path}> outside the system layer ({system})',
	lambda i, c: at(i.li, 0, path=i.path, system=c.layout.system_layer)
	if not i.quoted and i.path.endswith('.h') and i.path not in FREESTANDING
	and c.layout.layer is not None and c.layout.layer != c.layout.system_layer else None)

rule('guard-open', ERROR, Guard, 'header must open with #ifndef <guard> / #define <guard>',
	lambda g, c: at(0, 0) if not g.opening else None)
rule('guard-define', ERROR, Guard, '#define `{define}` does not match #ifndef `{guard}`',
	lambda g, c: at(g.directives[1][0], 0, define=g.directives[1][2], guard=g.name)
	if g.opening and g.directives[1][2] != g.name else None)
rule('guard-name', ERROR, Guard, 'guard `{guard}`, expected `{expected}`',
	lambda g, c: at(g.directives[0][0], 0, guard=g.name, expected=g.expected)
	if g.opening and g.expected and g.name != g.expected else None)
rule('guard-format', ERROR, Guard, 'guard `{guard}` must be lowercase ca_..._hpp',
	lambda g, c: at(g.directives[0][0], 0, guard=g.name)
	if g.opening and not (g.expected and g.name != g.expected)
	and not re.fullmatch(r'ca_[a-z0-9_]+_hpp', g.name) else None)


@rule('guard-close', ERROR, Guard, 'header must close with `#endif // {guard}`')
def _(g, c):
	if not g.opening:
		return None
	li, kw, _ = g.directives[-1]
	if kw != 'endif' or not re.fullmatch(rf'\s*#\s*endif // {re.escape(g.name)}\s*', c.src.raw[li]):
		return at(li, 0, guard=g.name)


# -- rules: lines ------------------------------------------------------------

rule('trailing-space', ERROR, Line, 'trailing whitespace',
	lambda l, c: at(l.li, len(l.raw.rstrip())) if l.raw.rstrip('\r') != l.raw.rstrip() else None)
rule('tabs', ERROR, Line, 'indent with tabs; spaces only after tabs',
	lambda l, c: at(l.li, 0) if not re.fullmatch(r'\t* *', l.raw[:len(l.raw) - len(l.raw.lstrip(' \t'))]) else None)
rule('no-std', ERROR, Line, 'std:: is banned', banned(r'\bstd\s*::'))
rule('no-exceptions', ERROR, Line, '`{token}`: exceptions are banned', banned(r'\b(throw|try|catch)\b'))
rule('no-rtti', ERROR, Line, '`{token}`: RTTI is banned', banned(r'\b(dynamic_cast|typeid)\b'))
rule('using-namespace', ERROR, Line, '`using namespace` is banned', banned(r'\busing\s+namespace\b'))
rule('literal-suffix', ERROR, Line, 'literal `{literal}`: uppercase suffix',
	lambda l, c: [at(l.li, m.start(), literal=m.group()) for m in LITERAL.finditer(l.code)
		if m.group(1) != m.group(1).upper()])
rule('template-line-space', ERROR, Line, 'write `template <`',
	lambda l, c: [at(l.li, s) for s, _ in l.templates if not l.code.startswith('template <', s)])
rule('template-line-decl', ERROR, Line, 'declaration must start on the line after `template <...>`',
	lambda l, c: [at(l.li, e + 1) for _, e in l.templates if e >= 0 and l.code[e + 1:].strip()])
rule('break-before-op', ERROR, Line, 'break before the operator, not after',
	lambda l, c: at(l.li, len(l.code.rstrip()) - 2) if re.search(r'(&&|\|\|)\s*$', l.code) else None)


def init_line(l):
	return not l.at.in_body and l.stripped.startswith(':') and not l.stripped.startswith('::')


rule('init-list-line', ERROR, Line, 'ctor init list starts on the next line with `: `',
	lambda l, c: at(l.li) if not l.at.in_body and not l.stripped.startswith(':')
	and INIT_SAME_LINE.search(l.code) and '?' not in l.code else None)
rule('init-list-space', ERROR, Line, 'init list line starts with `: ` (one space)',
	lambda l, c: at(l.li) if init_line(l) and not re.match(r': \S', l.stripped) else None)
rule('init-list-brace', ERROR, Line, 'brace-init `{name}{{...}}`, not parentheses',
	lambda l, c: [at(l.li, name=e.group(1)) for e in
		re.finditer(r'(?:^:|,)\s*([A-Za-z_][\w:]*(?:<[^>]*>)?)\s*\(', top_level(l.stripped))]
	if init_line(l) else None)


@rule('qualify', ERROR, Line, f'`{{name}}` must be written `{NAMESPACE}::{{name}}`')
def _(l, c):
	code = l.code
	for m in IDENT.finditer(code):
		w = m.group()
		if w not in c.index:
			continue
		before = code[:m.start()]
		b = before.rstrip()
		after = code[m.end():].lstrip()
		trailing = bool(re.search(r'\s->\s*$', before))
		if re.search(r'\b(class|struct|union)\s*(?:\[\[.*?\]\]\s*)+$', b):
			continue
		if b.endswith(('::', '.', '~')) or (b.endswith('->') and not trailing):
			continue
		if b.endswith('>') and not trailing:
			continue
		pm = re.search(r'([A-Za-z_]\w*)\s*$', b)
		if pm and pm.group(1) not in FLAG_AFTER:
			continue
		if w == l.at.cls and after.startswith('('):
			continue
		typed = re.match(r'[(<{]|::|\.\.\.|[&*]*\s*[A-Za-z_]', after)
		if not (typed or trailing or re.search(r'\busing\s+\w+\s*=$', b)):
			continue
		yield at(l.li, m.start(), name=w)


rule('indent-close', ERROR, Line, '`}}` at {actual} tabs, expected {expected}',
	lambda l, c: at(l.li, 0, actual=l.tabs, expected=l.at.indent)
	if l.stripped.startswith('}') and l.tabs != l.at.indent else None)
rule('indent-init', ERROR, Line, 'init list / base clause at the declaration indent',
	lambda l, c: at(l.li, 0) if not l.stripped.startswith('}') and l.cont and init_line(l)
	and l.stmt_indent is not None and l.tabs != l.stmt_indent else None)


@rule('indent-scope', ERROR, Line, '{actual} tabs, expected {expected}')
def _(l, c):
	if l.stripped.startswith('}') or l.cont or l.at.kind == 'switch':
		return None
	expected = l.at.inner
	if l.at.kind == 'class' and ACCESS.match(l.stripped):
		expected -= 1
	elif l.has_template and c.opts.template_indent == 'nested':
		expected += 1
	if l.tabs != expected:
		return at(l.li, 0, actual=l.tabs, expected=expected)


@rule('indent-comment', ERROR, CommentLine, 'comment at {actual} tabs, expected {expected}')
def _(cl, c):
	if cl.cont or cl.at.kind == 'switch':
		return None
	allowed = {cl.at.inner, cl.at.inner - 1} if cl.at.kind == 'class' else {cl.at.inner}
	if cl.tabs not in allowed:
		return at(cl.li, 0, actual=cl.tabs, expected=cl.at.inner)


# -- rules: comments ---------------------------------------------------------

rule('comment-in-body', ERROR, Comment, 'comment inside function body',
	lambda m, c: at(m.li, m.col) if m.at.in_body else None)
rule('name-echo', ERROR, Comment, 'name-echo comment `{body}`',
	lambda m, c: at(m.li, m.col, body=m.body) if m.block and re.fullmatch(r'/\*\s*[\w:~<>]+\s*\*/', m.body) else None)
rule('banner-format', ERROR, Comment, 'banner must be `// -- TITLE ---...`',
	lambda m, c: at(m.li, m.col) if m.is_banner and m.title is None else None)
rule('banner-width', ERROR, Comment, f'banner ends at column {{width}}, expected {BANNER_WIDTH}',
	lambda m, c: at(m.li, m.col, width=w) if m.is_banner and m.title is not None
	and (w := visual(m.lead, c.opts.tab_width) + len(m.body)) != BANNER_WIDTH else None)
rule('banner-spaced', ERROR, Comment, 'spaced banner title `{title}`; run without --no-fix to squash',
	lambda m, c: at(m.li, m.col, title=m.title) if m.is_banner and m.title and is_spaced(m.title) else None)
rule('banner-case', ERROR, Comment, 'class-level banner `{title}` must be lowercase',
	lambda m, c: at(m.li, m.col, title=m.title) if m.is_banner and m.title and m.at.kind == 'class'
	and not LOWER_TITLE.fullmatch(m.title) else None)
rule('banner-caps', ERROR, Comment, 'banner `{title}` must be capitals (`CA NAME`)',
	lambda m, c: at(m.li, m.col, title=m.title) if m.is_banner and m.title and m.at.kind != 'class'
	and not CAPS.fullmatch(m.title) else None)


@rule('section-order', ERROR, Comment, 'section `{title}` after `{previous}`')
def _(m, c):
	if not m.is_banner or m.at.kind != 'class' or m.title not in SECTION_RANK:
		return None
	known = [t for t in m.at.sections if t in SECTION_RANK]
	if known and SECTION_RANK[m.title] < SECTION_RANK[known[-1]]:
		return at(m.li, m.col, title=m.title, previous=known[-1])


# -- rules: braces and bodies ------------------------------------------------

rule('brace-unmatched', ERROR, Brace, 'unmatched `}}`',
	lambda b, c: at(b.li, b.col) if b.unmatched else None)
rule('brace-unclosed', ERROR, Brace, 'unclosed `{{` ({kind})',
	lambda b, c: at(b.li, b.col, kind=b.kind) if not b.unmatched else None)


def class_close(k):
	return k.kind == 'class' and k.name and not k.in_body


rule('close-comment-class', WARNING, Close, 'close with `}}; // {keyword} {name}`',
	lambda k, c: at(k.li, k.col, keyword=k.keyword, name=base)
	if class_close(k) and not re.fullmatch(
		rf'\s*;\s*// {k.keyword} {re.escape(base := k.name.split("::")[-1])}\s*', k.rest) else None)
rule('close-comment-namespace', WARNING, Close, 'close with `}} {want}`',
	lambda k, c: at(k.li, k.col, want=want) if k.kind == 'namespace'
	and not re.fullmatch(rf'\s*{re.escape(want := f"// namespace {k.name}".rstrip())}\s*', k.rest) else None)
rule('self', ERROR, Close, '{keyword} `{name}` lacks `using self = ...;`',
	lambda k, c: at(k.open_li, None, keyword=k.keyword, name=k.name) if class_close(k) and not k.has_self else None)

rule('body-spacing-inline', ERROR, Body, 'empty body: `{{` newline `}}`',
	lambda b, c: at(b.li, b.open_col) if b.inline and b.inner_empty else None)
rule('body-spacing-empty', ERROR, Body, 'empty body: `{{` newline `}}`, nothing between',
	lambda b, c: at(b.first, 0) if not b.inline and b.statements == 0 and b.first < b.li else None)
rule('body-spacing-open', ERROR, Body, 'multi-statement body needs a blank line after `{{`',
	lambda b, c: at(b.open_li) if not b.inline and b.statements >= 2 and not b.first_blank else None)
rule('body-spacing-single', ERROR, Body, 'no blank line in a one-statement body',
	lambda b, c: at(b.first, 0) if not b.inline and b.statements == 1 and b.first < b.li and b.first_blank else None)


def align_target(g, c):
	return max(visual(lead, c.opts.tab_width) for _, _, lead, _ in g.items)


rule('align-column', ERROR, AlignGroup, '`=` at column {column}, group aligns at {target}',
	lambda g, c: [at(li, None, column=col + 1, target=t + 1) for li, _, lead, _ in g.items
		if (col := visual(lead, c.opts.tab_width)) != (t := align_target(g, c))])
rule('align-space', ERROR, AlignGroup, 'right-align assignment targets: one space before `=`',
	lambda g, c: [at(li) for li, kind, lead, space in g.items
		if kind == 'assign' and space != ' ' and visual(lead, c.opts.tab_width) == align_target(g, c)])

# -- rules: declarations -----------------------------------------------------

rule('template-typename', ERROR, Decl, '`typename`, not `class`, in template parameters',
	lambda d, c: [at(d.li) for h in d.headers if re.search(r'(?<!enum )\bclass\b', h[h.find('<') + 1:-1])])


@rule('ns-leak', ERROR, Decl, f'{{what}} outside namespace {NAMESPACE}')
def _(d, c):
	hpp = c.src.ext == '.hpp'
	if d.what == 'namespace':
		if d.at.kind == 'file' and d.name.split('::')[0] != NAMESPACE and not (not hpp and d.name == ''):
			return at(d.li, what=f'namespace `{d.name or "<anonymous>"}`')
		return None
	if not d.at.file_scope:
		return None
	if d.head:
		return at(d.li, what=f'{d.what} `{d.full}`')
	if d.what in ('alias', 'using'):
		return at(d.li, what='using')
	if d.what in ('concept', 'class', 'struct', 'union', 'enum'):
		return at(d.li, what=f'{d.what} `{d.name}`')
	if d.what == 'function' and (hpp or (d.parts[0] != NAMESPACE and 'static' not in d.words)):
		return at(d.li, what=f'function `{d.full}`')
	if d.what == 'variable' and (hpp or 'static' not in d.words):
		return at(d.li, what=f'variable `{d.name}`')


@rule('naming', ERROR, Decl, '{what} `{name}` not snake_case')
def _(d, c):
	if d.what == 'namespace':
		return [at(d.li, what='namespace', name=p) for p in d.name.split('::') if p and not SNAKE.fullmatch(p)]
	if d.what == 'variable':
		if d.at.kind == 'class':
			bad = not static_constexpr(d) and d.name.startswith('_') and not SNAKE.fullmatch(d.name)
			return at(d.li, what='member', name=d.name) if bad else None
		if not d.at.file_scope and not (SNAKE.fullmatch(d.name) or UPPER.fullmatch(d.name)):
			return at(d.li, what='variable', name=d.name)
		return None
	if d.what == 'function' and (d.special or d.at.kind == 'linkage'):
		return None
	if d.what in ('alias', 'concept', 'function', 'class', 'struct', 'union', 'enum') and d.name:
		if not SNAKE.fullmatch(d.name):
			return at(d.li, what=d.what, name=d.name)


rule('concept-name', WARNING, Decl, 'concept `{name}` not named is_x / are_x / has_x',
	lambda d, c: at(d.li, name=d.name) if d.what == 'concept' and SNAKE.fullmatch(d.name)
	and not re.match(r'(is|are|has)_', d.name) else None)
rule('final', WARNING, Decl, '{keyword} `{name}` not final',
	lambda d, c: at(d.li, keyword=d.what, name=d.name) if d.head and d.name and d.final is False else None)


def static_constexpr(d):
	return 'static' in d.words and 'constexpr' in d.words


def member(d):
	return d.what == 'variable' and d.at.kind == 'class'


rule('member-prefix', ERROR, Decl, 'member `{name}` must be prefixed _',
	lambda d, c: at(d.li, name=d.name) if member(d) and not static_constexpr(d) and not d.name.startswith('_') else None)
rule('const-name', WARNING, Decl, 'class constant `{name}` must be UPPER_CASE or _name',
	lambda d, c: at(d.li, name=d.name) if member(d) and static_constexpr(d)
	and not (UPPER.fullmatch(d.name) or SNAKE.fullmatch(d.name) and d.name.startswith('_') or d.name == 'value') else None)
rule('file-state', WARNING, Decl, 'file-local `{name}` must be static constinit thread_local',
	lambda d, c: at(d.li, name=d.name) if d.what == 'variable' and d.at.file_scope and c.src.ext != '.hpp'
	and 'static' in d.words and 'constexpr' not in d.words
	and not ('constinit' in d.words and 'thread_local' in d.words) else None)


def function(d):
	return d.what == 'function'


rule('cpp-qualified', ERROR, Decl, f'define `{{name}}` at file scope as `{NAMESPACE}::...{{base}}`',
	lambda d, c: at(d.li, name=d.full, base=d.name) if function(d) and c.src.ext == '.cpp' and d.definition
	and d.at.kind == 'namespace' and d.at.ns and d.at.ns[0] else None)
rule('void-params', ERROR, Decl, '`{name}()` must be `{name}(void)`',
	lambda d, c: at(d.li, name=d.full) if function(d) and d.fn.params == '' else None)
rule('trailing-return', ERROR, Decl, '`{name}` must be `auto {base}(...) -> T`',
	lambda d, c: at(d.li, name=d.full, base=d.name) if function(d) and not (d.is_ctor or d.is_dtor or d.is_conv)
	and ([w for w in d.words if w not in SPEC_WORDS] != ['auto'] or '->' not in d.fn.rest) else None)
rule('noexcept', ERROR, Decl, '`{name}` lacks noexcept',
	lambda d, c: at(d.li, name=d.full) if function(d) and not re.search(r'=\s*delete\b', d.fn.rest)
	and not re.search(r'\bnoexcept\b', d.fn.rest) else None)
rule('private-prefix', ERROR, Decl, 'private method `{name}` must be prefixed _',
	lambda d, c: at(d.li, name=d.name) if function(d) and d.at.kind == 'class' and d.at.access == 'private'
	and not d.special and 'friend' not in d.words and not d.name.startswith('_') else None)
rule('nodiscard', WARNING, Decl, 'fallible `{name}` lacks [[nodiscard]]',
	lambda d, c: at(d.li, name=d.name) if function(d) and not d.nodiscard
	and (r := re.search(r'->\s*(.*)', d.fn.rest)) and re.search(r'\b(result|optional)\s*<', r.group(1)) else None)

rule('one-entity-missing', WARNING, Entities, f'no `{NAMESPACE}::{{stem}}` declared in {{stem}}.hpp',
	lambda e, c: at(0, 0, stem=e.stem) if e.header and e.names and e.stem not in e.names else None)
rule('one-entity-extra', WARNING, Entities, 'extra entities in {stem}.hpp: {extra}',
	lambda e, c: at(0, 0, stem=e.stem, extra=', '.join(sorted(e.names - {e.stem})))
	if e.header and e.names - {e.stem} else None)

# -- declaration parsing -----------------------------------------------------


def parse_namespace(name, scope, li):
	return Decl(li, scope, 'namespace', name=name, full=name)


def parse_class_head(t, scope, li, k):
	headers, rest = strip_templates(t)
	m = CLASS_HEAD.match(strip_attrs(rest))
	name = k.name
	return Decl(li, scope, k.keyword, name=name.split('::')[-1] if name else '', full=name,
		headers=tuple(headers), head=True, final=bool(m.group(4)) if m else None)


def parse_statement(t, scope, li, definition):
	headers, rest = strip_templates(t)
	nodiscard = 'nodiscard' in t
	rest = strip_attrs(rest)
	if not rest or (re.match(r'(class|struct|union|enum)\b', rest) and '{}' in rest):
		return None
	d = Decl(li, scope, 'other', headers=tuple(headers), definition=definition, nodiscard=nodiscard)
	word = IDENT.match(rest)
	word = word.group() if word else ''
	if word == 'using':
		if not re.match(r'using\s+namespace\b', rest):
			m = re.match(r'using\s+(\w+)\s*=', rest)
			d.what, d.name = ('alias', m.group(1)) if m else ('using', '')
	elif word in ('static_assert', 'typedef', 'namespace') or (word == 'friend' and '(' not in rest):
		pass
	elif word == 'concept':
		m = re.match(r'concept\s+(\w+)', rest)
		if m:
			d.what, d.name = 'concept', m.group(1)
	elif word in ('class', 'struct', 'union', 'enum'):
		m = re.match(r'(?:enum\s+(?:class|struct)\s+|\w+\s+)(\w+)', rest)
		if m:
			d.what, d.name = word, m.group(1)
	else:
		fn = split_function(rest)
		if fn:
			parse_function(d, fn)
		else:
			parse_variable(d, rest)
	return d


def parse_function(d, fn):
	opi = fn.name.find('operator')
	if opi >= 0:
		parts = [p.strip() for p in fn.name[:opi].split('::') if p.strip()] + [fn.name[opi:].strip()]
	else:
		parts = [p.strip() for p in fn.name.split('::')]
	base = parts[-1]
	if base.startswith('ca_') or (not fn.specs and fn.rest.startswith('->')):
		return
	scope = d.at
	cls = scope.name.split('::')[-1] if scope.kind == 'class' else (re.sub(r'<.*', '', parts[-2]) if len(parts) > 1 else None)
	d.what, d.name, d.full, d.fn, d.parts = 'function', base, fn.name, fn, tuple(parts)
	d.words = tuple(fn.specs.split())
	d.is_dtor = base.startswith('~')
	d.is_ctor = cls is not None and re.sub(r'<.*', '', base) == cls
	d.is_op = base.startswith('operator')
	d.is_conv = bool(re.match(r'operator\s+(?!new\b|delete\b)[A-Za-z_]', base))


def parse_variable(d, rest):
	m = re.match(r'([^(]*)\(\s*[*&]\s*([A-Za-z_]\w*)\s*\)\s*\(', rest)
	if m:
		d.what, d.name, d.full, d.words = 'variable', m.group(2), m.group(2), tuple(m.group(1).split())
		return
	decl = re.split(r'=|\{|\[|(?<!:):(?!:)', rest, maxsplit=1)[0]
	m = re.search(r'([A-Za-z_]\w*)\s*$', decl)
	if m:
		d.what, d.name, d.full, d.words = 'variable', m.group(1), m.group(1), tuple(decl.split())


# -- scope walk --------------------------------------------------------------


class Scope:
	"""A brace scope with the statement text accumulated since its last boundary."""

	_ids = itertools.count()

	def __init__(self, kind, parent=None, name='', keyword='', indent=0, line=0, col=0, lambda_=False, header=''):
		self.id = next(Scope._ids)
		self.kind = kind
		self.parent = parent
		self.name = name
		self.keyword = keyword
		self.indent = indent
		self.inner = 0 if kind == 'file' else indent + (2 if kind == 'class' else 1)
		self.line = line
		self.col = col
		self.lambda_ = lambda_
		self.header = header
		self.access = 'private' if keyword == 'class' else 'public'
		self.buf = []
		self.paren = 0
		self.stmt_line = None
		self.statements = 0
		self.has_self = False
		self.sections = []
		self.in_body = kind in BODY_KINDS or (parent is not None and parent.in_body)
		ns = parent.ns if parent else ()
		if kind == 'namespace':
			ns = ns + (tuple(name.split('::')) if name else ('',))
		self.ns = ns

	def push(self, ch, li):
		if self.stmt_line is None and not ch.isspace():
			self.stmt_line = li
		self.buf.append(ch)

	def text(self):
		return ''.join(self.buf)

	def clear(self):
		self.buf = []
		self.stmt_line = None

	def enclosing_class(self):
		s = self
		while s is not None:
			if s.kind == 'class':
				return s.name
			s = s.parent
		return None

	def at(self):
		return At(self.kind, self.name, self.keyword, self.indent, self.inner, self.in_body, self.access,
			self.ns, self.enclosing_class(), tuple(self.sections))

	def continuation(self):
		t = self.text().strip()
		if self.paren > 0:
			return True, False
		if not t:
			return False, False
		only, has_template = prefix_only(t)
		return not only, has_template


@dataclass(frozen=True)
class Kind:
	"""What a `{` opens."""
	kind: str
	name: str = ''
	keyword: str = ''
	lambda_: bool = False


def classify(s, t):
	_, rest = strip_templates(t)
	rest = strip_attrs(rest)
	bare = re.sub(r'\s*\[\[.*?\]\]', '', t)
	lambda_ = bool(LAMBDA.search(bare)) and not re.search(r'\boperator\s*\[\s*\]', t)
	if s.in_body:
		if lambda_:
			return Kind('block', lambda_=True)
		m = CLASS_HEAD.match(rest)
		if m and '(' not in rest:
			return Kind('class', m.group(2) or '', m.group(1))
		if re.match(r'enum\b', rest):
			return Kind('enum')
		if re.match(r'switch\b', t):
			return Kind('switch')
		if (not bare or bare.endswith(')') or re.search(r'\b(else|do|try)$', bare)
				or re.search(r'\)\s*(const|noexcept|mutable)$', bare)):
			return Kind('block')
		return Kind('init')
	m = re.match(r'(?:inline\s+)?namespace\b\s*([\w:]*)', t)
	if m:
		return Kind('namespace', m.group(1))
	if re.fullmatch(r'extern\s*"[^"]*"', t):
		return Kind('linkage')
	if re.match(r'enum\b', rest):
		return Kind('enum')
	m = CLASS_HEAD.match(rest)
	if m:
		return Kind('class', m.group(2) or '', m.group(1))
	if lambda_:
		return Kind('function', lambda_=True)
	if not t or re.search(r'(=|,|\(|\breturn)$', t):
		return Kind('init')
	if '(' in rest:
		head = re.sub(r'operator\s*\S+', '', rest[:rest.find('(')])
		if '=' in head:
			return Kind('init')
		if init_colon(rest) and re.search(r'[\w>]$', rest):
			return Kind('init')
		return Kind('function')
	return Kind('init')


class Model:
	"""A parsed file: its ordered events, suppressions and declared entities."""

	def __init__(self, src, layout):
		self.src = src
		self.layout = layout
		self.events = []
		self.suppress = {}
		self.entities = set()

	@classmethod
	def parse(cls, src, root=None):
		m = cls(src, Layout.of(src.path, root))
		Walker(m).run()
		return m

	def muted(self, li, r):
		names = self.suppress.get(li)
		if not names or r.name == 'directive':
			return False
		return SUPPRESS_ALL in names or any(r.selected_by(n) for n in names)


class Walker:
	"""Walks a file character by character, tracking scopes and emitting events."""

	def __init__(self, model):
		self.m = model
		self.src = model.src
		self.raw = model.src.raw
		self.emit = model.events.append
		self.align = []
		self.directives = []
		self.saw_system = False
		self.last_layer = 0
		self.pending = defaultdict(list)
		self.comments = defaultdict(list)
		for li, col, body, block in self.src.comments:
			sm = None if block else SUPPRESSION.fullmatch(body)
			if sm:
				names = tuple(sorted({n.strip() for n in sm.group(1).split(',')}))
				model.suppress.setdefault(li, set()).update(names)
				self.pending[li].append(Suppression(li, col, names))
			else:
				self.comments[li].append((col, body, block))

	def run(self):
		self.emit(File())
		stack = [Scope('file')]
		for li, code in enumerate(self.src.code):
			for e in self.pending.get(li, ()):
				self.emit(e)
			if self.src.pp[li]:
				if code.lstrip().startswith('#'):
					self.directive(li, code)
				continue
			s = stack[-1]
			comments = self.comments.get(li, [])
			if not code.strip():
				if comments:
					self.emit(CommentLine(li, tabs(self.raw[li]), s.at(), s.continuation()[0]))
					for c in comments:
						self.comment(li, c, s)
				continue
			self.line(li, code, s)
			ccols = {c[0]: c for c in comments}
			for col, ch in enumerate(code):
				s = stack[-1]
				if col in ccols:
					self.comment(li, ccols[col], s)
				if ch in '([':
					s.paren += 1
					s.push(ch, li)
				elif ch in ')]':
					s.paren = max(0, s.paren - 1)
					s.push(ch, li)
				elif ch == '{':
					self.open_brace(stack, li, col)
				elif ch == '}':
					self.close_brace(stack, li, col)
				elif ch == ';' and s.paren == 0:
					self.end_statement(s, li)
				elif ch == ',' and s.paren == 0 and s.kind in ('enum', 'init'):
					s.clear()
				elif ch == ':' and s.paren == 0 and self.is_label(s, code, col):
					s.clear()
				else:
					s.push(ch, li)
			stack[-1].push(' ', li)
		for s in stack[1:]:
			self.emit(Brace(s.line, s.col, s.kind, False))
		self.finish()

	def finish(self):
		if self.src.ext == '.hpp':
			self.emit(Guard(tuple(self.directives), self.m.layout.guard()))
		group = []
		for item in self.align + [None]:
			if item and group and group[-1][0] == item[0] - 1 and group[-1][1] == item[1]:
				group.append(item)
				continue
			if len(group) >= 2:
				self.emit(AlignGroup([(li, key[0], lead, space) for li, key, lead, space in group]))
			group = [item]
		stem = os.path.splitext(os.path.basename(self.src.path))[0]
		self.emit(Entities(stem, self.src.ext == '.hpp', self.m.entities))

	def directive(self, li, code):
		dm = re.match(r'\s*#\s*(\w*)\s*(.*)', code)
		if not dm:
			return
		kw, arg = dm.group(1), dm.group(2).strip()
		self.directives.append((li, kw, arg))
		self.emit(Directive(li, kw, arg))
		im = INCLUDE.match(self.raw[li]) if kw == 'include' else None
		if not im:
			return
		quoted, path = im.group(1) == '"', im.group(2)
		lm = re.match(r'(\d+)-[^/]+/', path) if quoted else None
		layer = int(lm.group(1)) if lm else None
		self.emit(Include(li, quoted, path, layer, self.last_layer, self.saw_system))
		if layer is not None:
			self.last_layer = max(self.last_layer, layer)
		if not quoted:
			self.saw_system = True

	def line(self, li, code, s):
		cont, has_template = s.continuation()
		stmt_indent = tabs(self.raw[s.stmt_line]) if s.stmt_line is not None else None
		self.emit(Line(li, code, self.raw[li], s.at(), cont, has_template, stmt_indent))
		if code.lstrip().startswith('}') or cont or s.kind == 'switch':
			return
		m = ALIAS.match(code) or (ASSIGN.match(code) if s.in_body else None)
		if m:
			kind = 'alias' if m.re is ALIAS else 'assign'
			self.align.append((li, (kind, s.id), self.raw[li][:m.end() - 1], m.group(1)))

	def comment(self, li, comment, s):
		col, body, block = comment
		self.emit(Comment(li, col, body, block, s.at(), self.raw[li][:col]))
		title = banner_title(body)
		if s.kind == 'class' and title:
			s.sections.append(title)

	def decl(self, s, d):
		if d is None:
			return
		if d.what == 'alias' and d.name == 'self' and s.kind == 'class':
			s.has_self = True
		if d.entity:
			self.m.entities.add(d.entity)
		self.emit(d)

	@staticmethod
	def is_label(s, code, col):
		if (col > 0 and code[col - 1] == ':') or (col + 1 < len(code) and code[col + 1] == ':'):
			return False
		t = s.text().strip()
		if s.kind == 'class' and t in ('public', 'private', 'protected'):
			s.access = t
			return True
		return bool(re.fullmatch(r'case\b.*|default', t))

	def open_brace(self, stack, li, col):
		s = stack[-1]
		t = s.text().strip()
		start = s.stmt_line if s.stmt_line is not None else li
		k = classify(s, t)
		if not s.in_body:
			if k.kind == 'class':
				self.decl(s, parse_class_head(t, s.at(), start, k))
			elif k.kind == 'function' and not k.lambda_:
				self.decl(s, parse_statement(t, s.at(), start, True))
			elif k.kind == 'namespace':
				self.decl(s, parse_namespace(k.name, s.at(), start))
		stack.append(Scope(k.kind, s, k.name, k.keyword, tabs(self.raw[li]), li, col, k.lambda_, t))

	def close_brace(self, stack, li, col):
		s = stack[-1]
		if s.kind == 'file':
			self.emit(Brace(li, col, 'file', True))
			return
		self.emit(Close(li, col, s.kind, s.name, s.keyword, s.in_body, s.has_self, s.line, self.raw[li][col + 1:]))
		if s.kind == 'function' and not s.lambda_:
			first = s.line + 1
			self.emit(Body(li, col, s.line, s.col, s.statements,
				s.line == li and not self.src.code[li][s.col + 1:col].strip(),
				first < len(self.raw) and not self.raw[first].strip()))
		stack.pop()
		p = stack[-1]
		if s.kind in ('class', 'enum', 'init') or s.lambda_:
			p.push('{', li)
			p.push('}', li)
		else:
			if p.in_body and s.kind in ('block', 'switch') and not re.match(r'else\b', s.header):
				p.statements += 1
			p.clear()

	def end_statement(self, s, li):
		t = s.text().strip()
		start = s.stmt_line if s.stmt_line is not None else li
		s.clear()
		if not t:
			return
		if s.in_body:
			s.statements += 1
			return
		if s.kind in ('enum', 'init'):
			return
		self.decl(s, parse_statement(t, s.at(), start, False))


def entities(path, text):
	"""Names a file declares directly in namespace ca (used by autodoc)."""
	return Model.parse(Source(path, text)).entities


# -- fixes -------------------------------------------------------------------


def squash_banners(text, tab):
	lines = text.split('\n')
	fixed = []
	for i, line in enumerate(lines):
		m = BANNER_LINE.match(line)
		if not m or not is_spaced(m.group(2)):
			continue
		title = ' '.join(w.replace(' ', '') for w in m.group(2).split('  '))
		head = f'{m.group(1)}// -- {title} '
		lines[i] = head + '-' * max(1, BANNER_WIDTH - visual(head, tab))
		fixed.append(i)
	return '\n'.join(lines), fixed


def write_atomic(path, text):
	fd, tmp = tempfile.mkstemp(dir=os.path.dirname(os.path.abspath(path)), prefix='.norm-')
	try:
		with os.fdopen(fd, 'w', encoding='utf-8', newline='') as fh:
			fh.write(text)
		shutil.copymode(path, tmp)
		os.replace(tmp, path)
	except BaseException:
		if os.path.exists(tmp):
			os.unlink(tmp)
		raise


# -- driver ------------------------------------------------------------------


class Reporter:
	"""Runs enabled rules over each model's events, printing diagnostics as found."""

	def __init__(self, opts, out=sys.stdout):
		self.opts = opts
		self.out = out
		self.errors = 0
		self.warnings = 0
		self.by_event = defaultdict(list)
		for r in RULES.values():
			if r.name in opts.enabled:
				self.by_event[r.on].append(r)

	def run(self, model, index):
		ctx = Ctx(model.src, model.layout, self.opts, index)
		raw = model.src.raw
		for event in model.events:
			for r in self.by_event.get(type(event), ()):
				for hit in r.hits(event, ctx):
					if model.muted(hit.li, r):
						continue
					col = hit.col
					if col is None:
						line = raw[hit.li] if hit.li < len(raw) else ''
						col = len(line) - len(line.lstrip())
					msg = r.message.format(**hit.fields)
					print(f'{model.src.path}:{hit.li + 1}:{col + 1}: {r.severity}: {msg} [{r.name}]', file=self.out)
					if r.severity == ERROR:
						self.errors += 1
					else:
						self.warnings += 1


@dataclass
class Options:
	enabled: frozenset
	root: str | None = None
	template_indent: str = 'same'
	tab_width: int = 4


def default_paths():
	repo = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
	for d in LIB_DIRS:
		if os.path.isdir(os.path.join(repo, d)):
			return [os.path.join(repo, d)]
	return [os.path.join(repo, LIB_DIRS[0])]


def collect(paths, exclude):
	files, missing, seen = [], [], set()

	def add(f):
		key = os.path.realpath(f)
		if key not in seen:
			seen.add(key)
			files.append(f)

	for p in paths:
		if os.path.isdir(p):
			for d, dirs, names in os.walk(p):
				dirs[:] = sorted(x for x in dirs if not x.startswith('.') and x not in exclude)
				for n in sorted(names):
					if n.endswith(SOURCE_EXT):
						add(os.path.join(d, n))
		elif os.path.isfile(p):
			add(p)
		else:
			missing.append(p)
	return files, missing


def load(path, opts, fix):
	"""Read and parse path, squashing spaced banners in place when fix; returns (Model, problem)."""
	try:
		with open(path, encoding='utf-8', newline='') as fh:
			text = fh.read()
	except (OSError, UnicodeDecodeError) as e:
		return None, str(e)
	if fix:
		squashed, fixed = squash_banners(text, opts.tab_width)
		if fixed:
			try:
				write_atomic(path, squashed)
			except OSError as e:
				return None, f'cannot rewrite banners: {e}'
			for i in fixed:
				print(f'{path}:{i + 1}: squashed banner', file=sys.stderr)
			text = squashed
	return Model.parse(Source(path, text), opts.root), None


def selection(arg, ap, flag):
	names = set()
	for sel in filter(None, (s.strip() for s in arg.split(','))):
		rules = select(sel)
		if not rules:
			ap.error(f'{flag}: unknown rule `{sel}`')
		names |= {r.name for r in rules}
	return names


def parse_args(argv):
	ap = argparse.ArgumentParser(description='Enforce the ca coding style (STYLE.md).')
	ap.add_argument('paths', nargs='*', help='files or directories (default: <repo>/src/lib)')
	ap.add_argument('--root', help='library root for layers and guards (default: nearest ancestor named lib)')
	ap.add_argument('--disable', default='', help='comma-separated rules or families to skip')
	ap.add_argument('--only', default='', help='comma-separated rules or families to run exclusively')
	ap.add_argument('--exclude', action='append', default=None,
		help=f'directory name to skip while walking, repeatable (default: {", ".join(EXCLUDE_DIRS)}; hidden dirs always skipped)')
	ap.add_argument('--werror', action='store_true', help='fail on warnings too')
	ap.add_argument('--no-fix', action='store_true', help='report spaced banners instead of rewriting files')
	ap.add_argument('--template-indent', choices=('same', 'nested'), default='same',
		help='declaration indent under `template <...>` (STYLE.md: same)')
	ap.add_argument('--tab-width', type=int, default=4, help='tab width for column checks')
	ap.add_argument('--list-rules', action='store_true', help='print rules and exit')
	args = ap.parse_args(argv)
	if args.tab_width < 1:
		ap.error('--tab-width must be positive')
	if args.root:
		args.root = os.path.abspath(args.root)
		if not os.path.isdir(args.root):
			ap.error(f'--root {args.root}: not a directory')
	enabled = selection(args.only, ap, '--only') if args.only else set(RULES)
	enabled -= selection(args.disable, ap, '--disable')
	args.options = Options(frozenset(enabled), args.root, args.template_indent, args.tab_width)
	return args


def main(argv=None):
	args = parse_args(argv)
	if args.list_rules:
		for r in RULES.values():
			print(f'{r.name:24} {r.severity:8} {r.on.__name__:12} {r.message}')
		return 0

	files, missing = collect(args.paths or default_paths(), set(args.exclude or EXCLUDE_DIRS))
	problems = [f'{p}: no such file or directory' for p in missing]
	models = []
	for f in files:
		model, problem = load(f, args.options, not args.no_fix)
		if problem:
			problems.append(f'{f}: {problem}')
		else:
			models.append(model)
	for p in problems:
		print(f'norm: {p}', file=sys.stderr)

	index = set(BUILTIN_NAMES).union(*(m.entities for m in models))
	reporter = Reporter(args.options)
	for model in models:
		reporter.run(model, index)
	print(f'norm: {reporter.errors} error(s), {reporter.warnings} warning(s) in {len(files)} file(s)', file=sys.stderr)
	if problems:
		return 2
	return 1 if reporter.errors or (args.werror and reporter.warnings) else 0


if __name__ == '__main__':
	sys.exit(main())
