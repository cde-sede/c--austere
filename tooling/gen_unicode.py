#!/usr/bin/env python3
"""Generate the ca::unicode character database: src/lib/2-vocabulary/unicode/impl/tables.hpp.

Usage: gen_unicode.py [--dry-run] [--ucd DIR] [--clean | --fclean]   (`make unicode`)
	--clean removes the build/ucd/ downloads, --fclean also tables.hpp (see cleanup.py).

Sources, both for the Unicode version of the running Python (unicodedata.unidata_version):
	classes         the running Python's str methods, one code point at a time: ca::unicode pins
	                Python semantics (str.isnumeric counts Unihan numerals, str.isidentifier takes
	                '_', ...). CASE_IGNORABLE, which Python keeps private, is read back from the
	                final sigma rule of str.lower.
	full mappings   str.upper, str.lower, str.title, str.casefold of one code point.
	simple mappings UnicodeData.txt (upper, lower, title) and CaseFolding.txt (C, S) of the UCD,
	                from --ucd DIR or downloaded once into build/ucd/<version>/. Python does not
	                expose them.
Python's full mappings are cross-checked against SpecialCasing.txt and CaseFolding.txt (F) over
the simple ones; a disagreement is an error (the UCD and Python versions differ).

Also src/lib/2-vocabulary/unicode/impl/class_ranges.hpp, what ca::regex compiles from: the classes
of Python's re as sorted code point ranges (\\w: str.isalnum or '_', \\d: str.isdecimal, \\s:
str.isspace), and its caseless classes, the code points re.IGNORECASE takes as one (same simple
lowercase, joined by re._casefix), each a cycle of caseless_link. Both are checked against the
running Python's re for every code point before writing.

Layout (Python's): block_of[point >> BLOCK_SHIFT] picks a block of record_index, whose entry
picks a record of records. BLOCK_SHIFT is the one, from 7 up (the ASCII fast path reads block 0
only), that makes the two index arrays smallest. TABLE_LIMIT ends the tables after the last block
holding a record other than records[0] (no class, maps to itself), used from there up. The tables
are decoded back for every code point and compared before writing.
"""

import argparse
import os
import re
import sys
import unicodedata
import urllib.request

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cleanup  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(REPO, 'src', 'lib', '2-vocabulary', 'unicode', 'impl', 'tables.hpp')
OUT_CLASSES = os.path.join(REPO, 'src', 'lib', '2-vocabulary', 'unicode', 'impl', 'class_ranges.hpp')
CACHE = os.path.join(REPO, 'build', 'ucd')
URL = 'https://www.unicode.org/Public/{version}/ucd/{name}'
UCD_FILES = ('UnicodeData.txt', 'SpecialCasing.txt', 'CaseFolding.txt')
WIDTH = 78
POINTS = 0x110000
MIN_SHIFT = 7
MAX_SHIFT = 12
MAX_MAPPING = 3
ROW = 16

FLAGS = (
	'ALPHA', 'DECIMAL', 'DIGIT', 'NUMERIC', 'SPACE', 'LINE_BREAK', 'UPPER', 'LOWER', 'TITLE',
	'CASED', 'CASE_IGNORABLE', 'PRINTABLE', 'ID_START', 'ID_CONTINUE', 'SPECIAL',
)
BIT = {name: 1 << i for i, name in enumerate(FLAGS)}
EMPTY = (0, 0, 0, 0, 0, 0)


# == sources =================================================================

def ucd_dir(opts, version):
	if opts.ucd:
		return opts.ucd
	path = os.path.join(CACHE, version)
	os.makedirs(path, exist_ok=True)
	for name in UCD_FILES:
		target = os.path.join(path, name)
		if os.path.exists(target):
			continue
		url = URL.format(version=version, name=name)
		print(f'  download {url}', file=sys.stderr)
		with urllib.request.urlopen(url, timeout=60) as response:
			data = response.read()
		with open(target + '.part', 'wb') as f:
			f.write(data)
		os.replace(target + '.part', target)
	return path


def ucd_lines(path, name):
	with open(os.path.join(path, name), encoding='utf-8') as f:
		for line in f:
			line = line.split('#', 1)[0].strip()
			if line:
				yield [field.strip() for field in line.split(';')]


def points_of(field):
	return [int(p, 16) for p in field.split()]


class Ucd:

	def __init__(self, path):
		self.upper = {}
		self.lower = {}
		self.title = {}
		self.fold = {}
		self.full_fold = {}
		self.special = {}
		self._unicode_data(path)
		self._case_folding(path)
		self._special_casing(path)

	def _unicode_data(self, path):
		for fields in ucd_lines(path, 'UnicodeData.txt'):
			point = int(fields[0], 16)
			upper, lower, title = fields[12], fields[13], fields[14]
			if upper:
				self.upper[point] = int(upper, 16)
			if lower:
				self.lower[point] = int(lower, 16)
			if title or upper:
				self.title[point] = int(title or upper, 16)

	def _case_folding(self, path):
		for fields in ucd_lines(path, 'CaseFolding.txt'):
			point, status, mapping = int(fields[0], 16), fields[1], points_of(fields[2])
			if status in ('C', 'S'):
				self.fold[point] = mapping[0]
			if status in ('C', 'F'):
				self.full_fold[point] = mapping

	def _special_casing(self, path):
		for fields in ucd_lines(path, 'SpecialCasing.txt'):
			if len(fields) > 4 and fields[4]:
				continue
			point = int(fields[0], 16)
			self.special[point] = (points_of(fields[3]), points_of(fields[1]), points_of(fields[2]))

	def simple(self, point):
		return (
			self.upper.get(point, point), self.lower.get(point, point),
			self.title.get(point, point), self.fold.get(point, point),
		)

	def full(self, point):
		upper, lower, title, _ = ([p] for p in self.simple(point))
		if point in self.special:
			upper, lower, title = self.special[point]
		return upper, lower, title, self.full_fold.get(point, [self.fold.get(point, point)])


def case_ignorable(c):
	after_cased = ('A' + c + 'Σ').lower()[-1] == 'ς'
	alone = (c + 'Σ').lower()[-1] == 'ς'
	before_end = ('AΣ' + c).lower()[1] == 'ς'
	return after_cased and before_end and not alone


def classes(c):
	upper, lower = c.isupper(), c.islower()
	title = unicodedata.category(c) == 'Lt'
	tests = (
		('ALPHA', c.isalpha()),
		('DECIMAL', c.isdecimal()),
		('DIGIT', c.isdigit()),
		('NUMERIC', c.isnumeric()),
		('SPACE', c.isspace()),
		('LINE_BREAK', len(('a' + c + 'b').splitlines()) > 1),
		('UPPER', upper),
		('LOWER', lower),
		('TITLE', title),
		('CASED', upper or lower or title),
		('CASE_IGNORABLE', case_ignorable(c)),
		('PRINTABLE', c.isprintable()),
		('ID_START', c.isidentifier()),
		('ID_CONTINUE', ('a' + c).isidentifier()),
	)
	flags = 0
	for name, value in tests:
		if value:
			flags |= BIT[name]
	return flags


class Database:

	def __init__(self, ucd):
		self.records = [EMPTY]
		self.specials = []
		self.point_record = [0] * POINTS
		self.mismatches = []
		record_ids = {EMPTY: 0}
		special_ids = {}
		for point in range(POINTS):
			c = chr(point)
			full = tuple(tuple(ord(p) for p in s) for s in (c.upper(), c.lower(), c.title(), c.casefold()))
			expected = tuple(tuple(m) for m in ucd.full(point))
			if full != expected:
				self.mismatches.append(point)
			simple = ucd.simple(point)
			flags = classes(c)
			special = 0
			if full != tuple((p,) for p in simple):
				if max(len(m) for m in full) > MAX_MAPPING:
					raise SystemExit(f'gen_unicode: U+{point:04X} maps to more than {MAX_MAPPING} code points')
				flags |= BIT['SPECIAL']
				special = special_ids.setdefault(full, len(special_ids))
				if special == len(self.specials):
					self.specials.append(full)
			record = (flags,) + tuple(s - point for s in simple) + (special,)
			index = record_ids.setdefault(record, len(record_ids))
			if index == len(self.records):
				self.records.append(record)
			self.point_record[point] = index


# == layout ==================================================================

def width_of(count):
	return 1 if count <= 0x100 else 2


def split(values, shift):
	size = 1 << shift
	blocks, block_ids, block_of = [], {}, []
	for start in range(0, len(values), size):
		block = tuple(values[start:start + size])
		index = block_ids.setdefault(block, len(block_ids))
		if index == len(blocks):
			blocks.append(block)
		block_of.append(index)
	return block_of, blocks


class Layout:

	def __init__(self, database):
		last = max(p for p in range(POINTS) if database.point_record[p] != 0)
		best = None
		for shift in range(MIN_SHIFT, MAX_SHIFT + 1):
			size = 1 << shift
			limit = (last // size + 1) * size
			block_of, blocks = split(database.point_record[:limit], shift)
			cost = len(block_of) * width_of(len(blocks)) + len(blocks) * size * width_of(len(database.records))
			if best is None or cost < best[0]:
				best = (cost, shift, limit, block_of, blocks)
		self.bytes, self.shift, self.limit, self.block_of, self.blocks = best

	def lookup(self, point):
		if point >= self.limit:
			return 0
		return self.blocks[self.block_of[point >> self.shift]][point & ((1 << self.shift) - 1)]


def verify(database, layout):
	for point in range(POINTS):
		if layout.lookup(point) != database.point_record[point]:
			raise SystemExit(f'gen_unicode: tables decode U+{point:04X} wrong')
	for record in database.records:
		flags, *_, special = record
		if (flags & BIT['SPECIAL']) == 0 and special != 0:
			raise SystemExit('gen_unicode: special index without SPECIAL')


# == regex classes ===========================================================

REGEX_CLASSES = (
	('word', r'\w', lambda c: c.isalnum() or c == '_'),
	('decimal', r'\d', lambda c: c.isdecimal()),
	('space', r'\s', lambda c: c.isspace()),
)


def ranges_of(test):
	ranges = []
	for point in range(POINTS):
		if not test(chr(point)):
			continue
		if ranges and ranges[-1][1] == point - 1:
			ranges[-1][1] = point
		else:
			ranges.append([point, point])
	return ranges


def verify_class(name, pattern, ranges):
	inside = [False] * POINTS
	for first, last in ranges:
		for point in range(first, last + 1):
			inside[point] = True
	compiled = re.compile(pattern)
	for point in range(POINTS):
		if (compiled.fullmatch(chr(point)) is not None) != inside[point]:
			raise SystemExit(f'gen_unicode: {name} ranges disagree with re {pattern} on U+{point:04X}')


def caseless_classes(ucd):
	parent = list(range(POINTS))

	def find(point):
		while parent[point] != point:
			parent[point] = parent[parent[point]]
			point = parent[point]
		return point

	def join(a, b):
		a, b = find(a), find(b)
		if a != b:
			parent[max(a, b)] = min(a, b)

	for point, lower in ucd.lower.items():
		join(point, lower)
	import re._casefix as casefix
	for point, others in casefix._EXTRA_CASES.items():
		for other in others:
			join(point, other)
	groups = {}
	for point in range(POINTS):
		groups.setdefault(find(point), []).append(point)
	return sorted(g for g in groups.values() if len(g) > 1)


def verify_caseless(classes):
	linked = {p for c in classes for p in c}
	for members in classes:
		for a in members:
			pattern = re.compile('(?i)' + re.escape(chr(a)))
			for b in members:
				if pattern.fullmatch(chr(b)) is None:
					raise SystemExit(f'gen_unicode: re.IGNORECASE does not take U+{a:04X} as U+{b:04X}')
	for point in range(POINTS):
		if point in linked:
			continue
		c = chr(point)
		pattern = None
		for other in (c.lower(), c.upper(), c.title(), c.casefold()):
			if len(other) != 1 or other == c:
				continue
			pattern = pattern or re.compile('(?i)' + re.escape(c))
			if pattern.fullmatch(other) is not None:
				raise SystemExit(f'gen_unicode: U+{point:04X} has a caseless twin outside the classes')


def caseless_links(classes):
	links = []
	for members in classes:
		for i, point in enumerate(members):
			links.append((point, members[(i + 1) % len(members)]))
	return sorted(links)


def emit_classes(classes, links, version):
	guard = 'ca_vocabulary_unicode_impl_class_ranges_hpp'
	lines = [f'#ifndef {guard}', f'#define {guard}', '']
	lines += ['#include "1-primitives/integers/u32.hpp"', '#include "2-vocabulary/unicode/impl/point_range.hpp"', '', '']
	lines += [banner(0, 'CA NAMESPACE'), '', 'namespace ca {', '', '']
	lines += ['\t' + banner(1, 'UNICODE')[1:], '', '\tnamespace unicode {', '', '']
	lines += ['\t\t' + banner(2, 'CLASS RANGES')[2:], '']
	lines += [
		f'\t\t/* Classes of Python\'s re for Unicode {version} as sorted, disjoint code point ranges, and',
		'\t\t   its caseless classes (re.IGNORECASE) as cycles of links sorted by point, generated by',
		'\t\t   tooling/gen_unicode.py (make unicode): do not edit. */',
		'\t\tnamespace impl {',
		'',
	]
	for name, ranges in classes:
		lines += [f'\t\t\tinline constexpr ca::u32 {name.upper()}_RANGE_COUNT = {len(ranges)}U;', '']
		lines += [f'\t\t\tinline constexpr ca::unicode::impl::point_range {name}_ranges[{name.upper()}_RANGE_COUNT] = {{']
		cells = [f'{{0x{a:04X}U, 0x{b:04X}U}}' for a, b in ranges]
		lines += ['\t\t\t\t' + ', '.join(cells[i:i + 6]) + ',' for i in range(0, len(cells), 6)]
		lines += ['\t\t\t};', '']
	lines += [f'\t\t\tinline constexpr ca::u32 CASELESS_LINK_COUNT = {len(links)}U;', '']
	lines += ['\t\t\tinline constexpr ca::unicode::impl::caseless_link caseless_links[CASELESS_LINK_COUNT] = {']
	cells = [f'{{0x{a:04X}U, 0x{b:04X}U}}' for a, b in links]
	lines += ['\t\t\t\t' + ', '.join(cells[i:i + 6]) + ',' for i in range(0, len(cells), 6)]
	lines += ['\t\t\t};', '', '\t\t} // namespace impl', '', '\t} // namespace unicode', '', '} // namespace ca', '']
	lines += [f'#endif // {guard}', '']
	return '\n'.join(lines)


# == emit ====================================================================

def banner(tabs, title):
	head = '\t' * tabs + f'// -- {title} '
	return head + '-' * (WIDTH - len(head.expandtabs(4)))


def integer_type(count):
	return 'ca::u8' if width_of(count) == 1 else 'ca::u16'


def rows(values, indent):
	cells = [f'{v}U' for v in values]
	return [indent + ', '.join(cells[i:i + ROW]) + ',' for i in range(0, len(cells), ROW)]


def flags_expression(flags):
	if flags == 0:
		return 'static_cast<ca::u16>(0U)'
	names = ' | '.join(f'ca::unicode::impl::char_record::{n}' for n in FLAGS if flags & BIT[n])
	return f'static_cast<ca::u16>({names})'


def emit_record(record):
	flags, upper, lower, title, fold, special = record
	return f'ca::unicode::impl::char_record{{{flags_expression(flags)}, {upper}, {lower}, {title}, {fold}, {special}U}},'


def emit_special(mappings):
	parts = []
	for mapping in mappings:
		padded = list(mapping) + [0] * (MAX_MAPPING - len(mapping))
		parts.append('{' + ', '.join(f'0x{p:04X}U' for p in padded) + '}')
	return f'ca::unicode::impl::special_case{{{", ".join(parts)}}},'


def emit(database, layout, version):
	guard = 'ca_vocabulary_unicode_impl_tables_hpp'
	block_type = integer_type(len(layout.blocks))
	index_type = integer_type(len(database.records))
	includes = sorted({block_type, index_type, 'ca::u32'})
	lines = [f'#ifndef {guard}', f'#define {guard}', '']
	lines += [f'#include "1-primitives/integers/{t[4:]}.hpp"' for t in includes]
	lines += ['#include "2-vocabulary/unicode/impl/char_record.hpp"', '', '']
	lines += [banner(0, 'CA NAMESPACE'), '', 'namespace ca {', '', '']
	lines += ['\t' + banner(1, 'UNICODE')[1:], '', '\tnamespace unicode {', '', '']
	lines += ['\t\t' + banner(2, 'TABLES')[2:], '']
	lines += [
		f'\t\t/* Character database of Unicode {version}, generated by tooling/gen_unicode.py (make',
		'\t\t   unicode): do not edit. Python\'s layout: block_of[point >> BLOCK_SHIFT] picks a block',
		'\t\t   of record_index, whose entry picks a record; points from TABLE_LIMIT up use records[0]',
		'\t\t   (no property, mapped to themselves). BLOCK_SHIFT is at least 7: ASCII is block 0. */',
		'\t\tnamespace impl {',
		'',
		f'\t\t\tinline constexpr ca::u32 TABLE_LIMIT = 0x{layout.limit:X}U;',
		'',
		f'\t\t\tinline constexpr ca::u32 BLOCK_SHIFT = {layout.shift}U;',
		'',
		f'\t\t\tinline constexpr {block_type} block_of[TABLE_LIMIT >> BLOCK_SHIFT] = {{',
	]
	lines += rows(layout.block_of, '\t\t\t\t')
	lines += ['\t\t\t};', '']
	lines += [f'\t\t\tinline constexpr {index_type} record_index[{len(layout.blocks)}][1U << BLOCK_SHIFT] = {{']
	for block in layout.blocks:
		lines += ['\t\t\t\t{']
		lines += rows(block, '\t\t\t\t\t')
		lines += ['\t\t\t\t},']
	lines += ['\t\t\t};', '']
	lines += ['\t\t\tinline constexpr ca::unicode::impl::char_record records[] = {']
	lines += ['\t\t\t\t' + emit_record(r) for r in database.records]
	lines += ['\t\t\t};', '']
	lines += [f'\t\t\tinline constexpr ca::u32 SPECIAL_CASE_COUNT = {len(database.specials)}U;', '']
	if database.specials:
		lines += ['\t\t\tinline constexpr ca::unicode::impl::special_case special_cases[SPECIAL_CASE_COUNT] = {']
		lines += ['\t\t\t\t' + emit_special(s) for s in database.specials]
		lines += ['\t\t\t};']
	else:
		lines += ['\t\t\tinline constexpr ca::unicode::impl::special_case special_cases[1] = {};']
	lines += ['', '\t\t} // namespace impl', '', '\t} // namespace unicode', '', '} // namespace ca', '']
	lines += [f'#endif // {guard}', '']
	return '\n'.join(lines)


def report(database, layout, version):
	print(f'gen_unicode: Unicode {version} (Python {sys.version.split()[0]})', file=sys.stderr)
	print(f'  {len(database.records)} records, {len(database.specials)} special cases', file=sys.stderr)
	print(
		f'  BLOCK_SHIFT {layout.shift}, TABLE_LIMIT 0x{layout.limit:X}, {len(layout.blocks)} blocks, '
		f'index arrays {layout.bytes} bytes', file=sys.stderr,
	)


def write(path, text, dry_run):
	rel = os.path.relpath(path, REPO)
	if dry_run:
		print(f'  would write {rel}', file=sys.stderr)
		return
	old = open(path).read() if os.path.exists(path) else None
	if old == text:
		print(f'  unchanged {rel}', file=sys.stderr)
		return
	with open(path, 'w') as f:
		f.write(text)
	print(f'  wrote {rel}', file=sys.stderr)


def main():
	ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
	ap.add_argument('--dry-run', action='store_true', help='report only, write nothing')
	ap.add_argument('--ucd', help='directory holding UnicodeData.txt, SpecialCasing.txt, CaseFolding.txt')
	cleanup.add_arguments(ap)
	opts = ap.parse_args()
	if opts.clean or opts.fclean:
		return cleanup.run(opts, 'gen_unicode', [CACHE] + cleanup.bytecode(), [OUT, OUT_CLASSES])

	version = unicodedata.unidata_version
	ucd = Ucd(ucd_dir(opts, version))
	database = Database(ucd)
	if database.mismatches:
		shown = ' '.join(f'U+{p:04X}' for p in database.mismatches[:16])
		raise SystemExit(f'gen_unicode: Python and the UCD disagree on {len(database.mismatches)} full mappings: {shown}')
	layout = Layout(database)
	verify(database, layout)
	report(database, layout, version)
	write(OUT, emit(database, layout, version), opts.dry_run)
	classes = []
	for name, pattern, test in REGEX_CLASSES:
		ranges = ranges_of(test)
		verify_class(name, pattern, ranges)
		classes.append((name, ranges))
	caseless = caseless_classes(ucd)
	verify_caseless(caseless)
	links = caseless_links(caseless)
	print(
		f'  regex classes: {", ".join(f"{n} {len(r)} ranges" for n, r in classes)}; '
		f'{len(caseless)} caseless classes, {len(links)} links', file=sys.stderr,
	)
	write(OUT_CLASSES, emit_classes(classes, links, version), opts.dry_run)


if __name__ == '__main__':
	sys.exit(main())
