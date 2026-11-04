#!/usr/bin/env python3
"""Writes tests/regex_python.cpp: ca::regex against CPython's re. Every expectation is what the
running Python returns for the same call, so the test pins Python semantics: a curated list of
cases, then random patterns and texts from a fixed seed (the differential test that found the
engine's divergences, kept as a regression net).

    tooling/gen_regex_test.py [--clean | --fclean]

Positions are compared in code units of the UTF-8 text (Python's character indices are
converted). A group that took no part is an empty view in findall and split (Python: None, or
''). Cases where Python itself errors are expected to fail in ca too ('error', 'suberror').
`\\B` on an empty text is left out: ca follows Python 3.14 (it matches there), Python 3.12 and
older do not. --clean leaves nothing to remove but bytecode; --fclean also removes
tests/regex_python.cpp (see cleanup.py).
"""

import argparse
import os
import random
import re
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cleanup  # noqa: E402

OUT = os.path.join(cleanup.REPO, 'tests', 'regex_python.cpp')
SEED = 20261103
RANDOM_CASES = 1500

CURATED = [
	('search', '', r'(\w+)@(?P<host>[a-z]+)\.com', 'mail bob@example.com now', ''),
	('search', '', 'abc', 'xxabcxx', ''),
	('search', '', 'abc', 'xxabxx', ''),
	('match', '', 'abc', 'xabc', ''),
	('fullmatch', '', 'a|ab', 'ab', ''),
	('match', '', 'a|ab', 'ab', ''),
	('search', '', 'a*?b', 'aaab', ''),
	('search', '', '(a+)(a*)', 'aaaa', ''),
	('search', '', '(a+?)(a*)', 'aaaa', ''),
	('search', '', 'a{2,3}', 'aaaa', ''),
	('search', '', 'a{2,3}?', 'aaaa', ''),
	('search', '', 'a{,2}b', 'aaab', ''),
	('search', '', 'a{2}', 'a', ''),
	('search', '', 'a{', 'xa{', ''),
	('search', '', 'a{x}', 'a{x}', ''),
	('search', '', 'a{,}', 'aaa', ''),
	('search', '', '^abc$', 'abc\n', ''),
	('search', '', 'abc$', 'abc\n\n', ''),
	('search', '', r'abc\Z', 'abc\n', ''),
	('findall', 'm', '^\\w+$', 'one\ntwo\n\nthree', ''),
	('findall', '', '^\\w+$', 'one\ntwo', ''),
	('search', '', 'a.c', 'a\nc', ''),
	('search', 's', 'a.c', 'a\nc', ''),
	('search', '', r'\bcat\b', 'concat cat category', ''),
	('findall', '', r'\b\w+\b', 'héllo wörld, ça va', ''),
	('findall', 'a', r'\b\w+\b', 'héllo wörld', ''),
	('findall', '', r'\B\w', 'ab cd', ''),
	('findall', '', r'\d+', 'a1b22c333 ٣٤ ٥', ''),
	('findall', 'a', r'\d+', 'a1b22c333 ٣٤ ٥', ''),
	('findall', '', r'\s+', 'a b\tc d e', ''),
	('findall', 'a', r'\s+', 'a b\tc d', ''),
	('findall', '', r'[^\W\d_]+', 'ab_12cd é3', ''),
	('search', 'i', 'k', 'K', ''),
	('search', 'i', 'K', 'k', ''),
	('search', 'i', 'K', 'k', ''),
	('search', 'ai', 'k', 'K', ''),
	('search', 'i', 's', 'ſ', ''),
	('search', 'i', 'i', 'İ', ''),
	('search', 'i', 'i', 'ı', ''),
	('search', 'i', '[a-z]+', 'HeLLo', ''),
	('search', 'i', '[^a-z]+', 'HeLLo123', ''),
	('search', 'i', 'σ', 'Σς', ''),
	('findall', 'i', 'σ', 'Σςσ', ''),
	('search', 'i', 'é', 'É', ''),
	('search', 'ai', 'é', 'É', ''),
	('search', '', '(?i)abc', 'ABC', ''),
	('search', '', '(?i:a)b', 'Ab', ''),
	('search', '', '(?i:a)b', 'AB', ''),
	('search', 'i', '(?-i:a)b', 'aB', ''),
	('search', 'i', '(?-i:a)b', 'AB', ''),
	('search', '', '(?s:.)', '\n', ''),
	('search', '', '(?m:^b)', 'a\nb', ''),
	('search', '', '(?x) a b c # comment', 'abc', ''),
	('search', 'x', r'a\ b [ ] # c', 'a b ', ''),
	('search', 'x', 'a  b\n# x\n c', 'abc', ''),
	('search', '', r'\x41é\U0001F600', 'Aé😀', ''),
	('search', '', r'\101\0', 'A\x00', ''),
	('search', '', r'[\101-\103]+', 'ABCD', ''),
	('search', '', r'[\b]', 'a\bb', ''),
	('search', '', r'\.\*\+\?', '.*+?', ''),
	('search', '', '[]a]+', ']a]', ''),
	('search', '', '[^]a]+', 'a]bc', ''),
	('search', '', '[a-]+', '-a-', ''),
	('search', '', '[-a]+', '-a-', ''),
	('search', '', '[\\w-]+', 'ab-c d', ''),
	('search', '', '[.]', 'a.b', ''),
	('search', '', '(?#comment)a', 'a', ''),
	('search', '', '😀+', 'x😀😀y', ''),
	('search', '', '[😀-😂]+', 'x😁😂y', ''),
	('search', '', '[^😀]', '😀a', ''),
	('search', '', '.', 'é', ''),
	('findall', '', '.', 'aé😀\n', ''),
	('findall', '', '', 'aé', ''),
	('findall', '', 'x*', 'axxb', ''),
	('finditer', '', 'x*', 'axxb', ''),
	('finditer', '', r'\b', 'ab cd', ''),
	('split', '', r'\W+', 'Words, words, words.', ''),
	('split', '', r'(\W+)', 'Words, words, words.', ''),
	('split1', '', r'\W+', 'Words, words, words.', ''),
	('split', '', 'x*', 'axbc', ''),
	('split', '', r'\b', 'a b', ''),
	('split', '', '(a)|(b)', 'xaybz', ''),
	('split', '', '[a-f]+', '0a3B9', ''),
	('split', 'i', '[a-f]+', '0a3B9', ''),
	('findall', '', r'(\w+)=(\d+)', 'a=1, b=22, c=x', ''),
	('findall', '', r'(\w+)=\d+', 'a=1, b=22', ''),
	('findall', '', r'(a)|(b)', 'ab', ''),
	('sub', '', 'x*', 'abxd', '-'),
	('sub', '', r'(\w+) (\w+)', 'hello world', r'\2 \1'),
	('sub', '', r'(?P<first>\w+) (?P<second>\w+)', 'hello world', r'\g<second> \g<first>'),
	('sub', '', r'(\w+)', 'ab', r'\g<1>0'),
	('sub', '', r'(\w+)', 'ab', r'\10'),
	('sub', '', r'a', 'aaa', r'\n\t\\'),
	('sub', '', r'a', 'a', r'\0\101\x'),
	('sub', '', r'a', 'a', r'\&'),
	('sub', '', r'a', 'a', r'\q'),
	('sub', '', r'(a)', 'a', r'\2'),
	('sub', '', r'(a)', 'a', r'\g<x>'),
	('sub', '', r'(a)', 'a', r'\g<1'),
	('sub', '', r'(a)|b', 'ab', r'[\1]'),
	('sub1', '', 'a', 'aaa', '-'),
	('sub', '', '', 'abc', '-'),
	('sub', '', r'\s+', '  a  b  ', ' '),
	('search', '', '(a|)*', 'aa', ''),
	('search', '', '(a|)+', 'aa', ''),
	('search', '', '(a*)*', 'b', ''),
	('search', '', '(a*)+', 'b', ''),
	('search', '', '(a|b)*', 'abab', ''),
	('search', '', '(?:(a)|b)*', 'ab', ''),
	('search', '', '(a?)*?b', 'aab', ''),
	('search', '', r'(\w*)*-', 'ab-', ''),
	('search', '', '((a)|(b))*', 'ab', ''),
	('search', '', '(a)?b', 'b', ''),
	('search', '', '(?P<x>a)(?P<y>b)?', 'a', ''),
	('search', '', '(', 'a', ''),
	('search', '', ')', 'a', ''),
	('search', '', '[a', 'a', ''),
	('search', '', '[z-a]', 'a', ''),
	('search', '', '*a', 'a', ''),
	('search', '', 'a**', 'a', ''),
	('search', '', r'\q', 'q', ''),
	('search', '', 'a{3,2}', 'a', ''),
	('search', '', '(?P<a>x)(?P<a>y)', 'xy', ''),
	('search', '', '(?P<1a>x)', 'x', ''),
	('search', '', '(?z)', 'x', ''),
	('search', '', 'a(?i)b', 'ab', ''),
	('search', '', '(?au)a', 'a', ''),
	('search2', '', '^a', 'aaa', ''),
	('search2', '', r'\ba', 'a aa', ''),
	('search2', '', 'a', 'aéa', ''),
	('match2', '', 'a+', 'baaa', ''),
	('fullmatch', '', r'\d+', '123', ''),
	('fullmatch', '', r'\d+', '123a', ''),
	('fullmatch', '', '(a*)(a)', 'aaa', ''),
	('fullmatch', '', '.*?', 'abc', ''),
	('match', '', '.*?', 'abc', ''),
	('search', '', 'a\x00b', 'xa\x00b', ''),
	('search', '', '\\\x00', '\x00', ''),
]

OPS = ['search', 'match', 'fullmatch', 'finditer', 'findall', 'split', 'sub']
FLAGS = ['', 'i', 'm', 's', 'a', 'im', 'ms', 'ai']
ATOMS = [
	'a', 'b', 'c', '.', '[ab]', '[^a]', r'\w', r'\W', r'\d', r'\s', r'\b', r'\B', '^', '$', r'\A', r'\Z', 'é', '[a-cé]', ' ',
	r'\n', 'x', '[^\\n]', 'A', '[A-Z]', 'k', 'K', 's', 'i', '(?i:k)', '(?-i:a)', '\\x41', '[^\\W\\d]', '\\S', '[\\w-]', '[\\s\\d]',
	'\\.', '[]a]', '[a-]', '(?#c)',
]
QUANT = ['*', '+', '?', '{2}', '{1,2}', '{0,1}', '{2,}', '*?', '+?', '??', '{1,2}?']
ALPHABET = ['a', 'b', 'c', 'A', 'B', ' ', '\n', '1', 'é', 'É', 'x', '_', '-', 'k', 'K', 'K', 's', 'ſ', 'i', 'İ', '.', ']']
REPLS = ['-', r'<\g<0>>', r'[\1]', r'\n', r'x\g<n1>y', r'\g<1>\g<0>', r'\&']


def pattern(rng, depth=0):
	kind = rng.random()
	if depth > 3 or kind < 0.35:
		return rng.choice(ATOMS)
	if kind < 0.55:
		return ''.join(pattern(rng, depth + 1) for _ in range(rng.randint(1, 3)))
	if kind < 0.7:
		return '|'.join(pattern(rng, depth + 1) for _ in range(rng.randint(2, 3)))
	if kind < 0.85:
		opener = rng.choice(['(', '(?:', '(?P<n%d>' % rng.randint(0, 999)])
		return opener + pattern(rng, depth + 1) + ')'
	inner = pattern(rng, depth + 1)
	if not inner.startswith('(') or inner[-1] in '*+?}' or inner in ('^', '$', r'\A', r'\Z', r'\b', r'\B'):
		inner = '(?:' + inner + ')'
	return inner + rng.choice(QUANT)


def random_cases():
	rng = random.Random(SEED)
	cases = []
	while len(cases) < RANDOM_CASES:
		text = ''.join(rng.choice(ALPHABET) for _ in range(rng.randint(0, 12)))
		case = (rng.choice(OPS), rng.choice(FLAGS), pattern(rng), text, rng.choice(REPLS))
		if text == '' and '\\B' in case[2]:
			continue
		cases.append(case)
	return cases


def offset(text, index):
	return len(text[:index].encode())


def span(m, text):
	if m is None:
		return 'none'
	out = f'{offset(text, m.start())},{offset(text, m.end())}'
	for g in range(1, m.re.groups + 1):
		out += ';-' if m.start(g) == -1 else f';{offset(text, m.start(g))},{offset(text, m.end(g))}'
	return out


def expected(op, flags, source, text, replacement):
	value = 0
	for c in flags:
		value |= {'i': re.I, 'm': re.M, 's': re.S, 'x': re.X, 'a': re.A}[c]
	try:
		compiled = re.compile(source, value)
	except re.error:
		return 'error'
	if op in ('search', 'match', 'fullmatch'):
		return span(getattr(compiled, op)(text), text)
	if op in ('search2', 'match2'):
		return span(getattr(compiled, op[:-1])(text, 2), text)
	if op == 'finditer':
		return ''.join('[' + span(m, text) + ']' for m in compiled.finditer(text))
	if op == 'findall':
		items = []
		for found in compiled.findall(text):
			items += list(found) if isinstance(found, tuple) else [found]
		return ''.join('[' + item + ']' for item in items)
	if op in ('split', 'split1'):
		return ''.join('[' + (piece or '') + ']' for piece in compiled.split(text, 1 if op == 'split1' else 0))
	try:
		return compiled.sub(replacement, text, 1 if op == 'sub1' else 0)
	except (re.error, IndexError):
		return 'suberror'


def literal(text):
	data = text.encode() if isinstance(text, str) else text
	out = '"'
	hexed = False
	for byte in data:
		c = chr(byte)
		if hexed and c in '0123456789abcdefABCDEF':
			out += '""'
		hexed = False
		if c in '"\\?':
			out += '\\' + c
		elif 32 <= byte < 127:
			out += c
		else:
			out += '\\x%02x' % byte
			hexed = True
	return out + '"'


def view(text):
	return f'{{{literal(text)}, {len(text.encode())}U}}'


def main():
	ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
	cleanup.add_arguments(ap)
	opts = ap.parse_args()
	if opts.clean or opts.fclean:
		return cleanup.run(opts, 'gen_regex_test', cleanup.bytecode(), [OUT])
	cases = CURATED + random_cases()
	rows = []
	for op, flags, source, text, replacement in cases:
		want = expected(op, flags, source, text, replacement)
		pos = offset(text, min(2, len(text))) if op.endswith('2') else 0
		rows.append(f'\t\t{{{literal(op)}, {literal(flags)}, {view(source)}, {view(text)}, {view(replacement)}, {view(want)}, {pos}U}},')
	body = TEMPLATE.replace('@ROWS@', '\n'.join(rows))
	old = open(OUT).read() if os.path.exists(OUT) else None
	if old != body:
		with open(OUT, 'w') as f:
			f.write(body)
	print(f'gen_regex_test: {len(cases)} cases ({len(CURATED)} curated), Python {sys.version.split()[0]}', file=sys.stderr)
	return 0


TEMPLATE = '''// expect: exit 0
// Generated by tooling/gen_regex_test.py: every expectation is what CPython's re returns.
#include "5-collections/format/format_append.hpp"
#include "5-collections/regex/regex.hpp"
#include "5-collections/string/str.hpp"
#include "6-resources/print/print.hpp"

namespace {

	struct text final {

			const char* _data;
			ca::usz     _size;

			[[nodiscard]] auto view(void) const noexcept -> ca::sv::str {
				return ca::sv::str{_data, _size};
			}

	}; // struct text

	struct regex_case final {

			const char* _op;
			const char* _flags;
			text        _pattern;
			text        _text;
			text        _replacement;
			text        _expected;
			ca::usz     _pos;

	}; // struct regex_case

	constexpr regex_case cases[] = {
@ROWS@
	};

	auto flags_of(const ca::sv::str letters) noexcept -> ca::regex_flags {

		ca::regex_flags flags = ca::regex_flags::none;
		for (const char c : letters) {
			if (c == 'i')
				flags = flags | ca::regex_flags::ignore_case;
			else if (c == 'm')
				flags = flags | ca::regex_flags::multiline;
			else if (c == 's')
				flags = flags | ca::regex_flags::dot_all;
			else if (c == 'x')
				flags = flags | ca::regex_flags::verbose;
			else if (c == 'a')
				flags = flags | ca::regex_flags::ascii;
		}
		return flags;
	}

	auto span(const ca::regex_match<>& match, ca::str& out) noexcept -> bool {

		bool ok = static_cast<bool>(ca::format_append(out, "{},{}", match.start(), match.end()));
		for (ca::usz group = 1U; group <= match.group_count() && ok; ++group) {
			const ca::optional<ca::usz> start = match.start(group);
			const ca::optional<ca::usz> end = match.end(group);
			ok = start && end ? static_cast<bool>(ca::format_append(out, ";{},{}", *start, *end)) : static_cast<bool>(out.append(";-"));
		}
		return ok;
	}

	auto one(ca::result<ca::optional<ca::regex_match<>>>&& found, ca::str& out) noexcept -> bool {

		if (!found)
			return false;
		if (!*found)
			return static_cast<bool>(out.append("none"));
		const bool ok = span(**found, out);
		return (*found)->cleanup() && ok;
	}

	auto views(ca::result<ca::dynamic_array<ca::sv::str>>&& found, ca::str& out) noexcept -> bool {

		if (!found)
			return false;
		bool ok = true;
		for (const ca::sv::str piece : *found)
			ok = ok && out.append("[") && out.append(piece) && out.append("]");
		return found->cleanup() && ok;
	}

	auto run(const regex_case& test, ca::str& out) noexcept -> bool {

		auto made = ca::regex::make(ca::regex_pattern{test._pattern.view()}, flags_of(test._flags));
		if (!made)
			return static_cast<bool>(out.append("error"));
		ca::regex& regex = *made;
		const ca::sv::str op{test._op};
		const ca::sv::str subject = test._text.view();
		bool ok = true;
		if (op == "search")
			ok = one(regex.search(subject), out);
		else if (op == "search2")
			ok = one(regex.search(subject, test._pos), out);
		else if (op == "match")
			ok = one(regex.match(subject), out);
		else if (op == "match2")
			ok = one(regex.match(subject, test._pos), out);
		else if (op == "fullmatch")
			ok = one(regex.fullmatch(subject), out);
		else if (op == "finditer") {
			ca::regex::matches_type matches = regex.finditer(subject);
			while (ok) {
				auto next = matches.next();
				if (!next || !*next) {
					ok = static_cast<bool>(next);
					break;
				}
				ok = out.append("[") && span(**next, out) && out.append("]") && (*next)->cleanup();
			}
		}
		else if (op == "findall")
			ok = views(regex.findall(subject), out);
		else if (op == "split" || op == "split1")
			ok = views(regex.split(subject, op == "split1" ? 1U : 0U), out);
		else {
			auto replaced = regex.sub(test._replacement.view(), subject, op == "sub1" ? 1U : 0U);
			ok = replaced ? out.append(replaced->view()) && replaced->cleanup() : static_cast<bool>(out.append("suberror"));
		}
		return regex.cleanup() && ok;
	}

} // namespace

auto main(void) -> int {

	int failed = 0;
	ca::str out{};
	ca::usz index = 0U;
	for (const regex_case& test : cases) {
		out.clear();
		if (!run(test, out) || out.view() != test._expected.view()) {
			static_cast<void>(ca::print("case {}: /{:?}/ on {:?}: {:?}, expected {:?}\\n", index, test._pattern.view(), test._text.view(), out.view(), test._expected.view()));
			++failed;
		}
		++index;
	}
	return out.cleanup() && failed == 0 ? 0 : 1;
}
'''

if __name__ == '__main__':
	sys.exit(main())
