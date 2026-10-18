"""Benchmark tables in /* @DOC */ comments, for autodoc.py --bench.

A @DOC quotes a benchmark between two marker lines:

	--BENCH bench/NAME.cpp [SECTION]
	...                                (generated: replaced on every --bench run)
	BENCH--

--bench builds the release library per compiler, builds bench/NAME.cpp once as the
baseline (-DUSE_STD) and once per variant the bench declares (`// variant: LABEL
[-DFLAG...]` lines; none means one variant, ca), runs each pinned to one core, and
rewrites the lines between the markers: a line saying where and how it was measured,
then a table with one row per case (only SECTION's when given), one column per
compiler and variant holding ca's user cycles divided by the baseline's, and the case
notes. Case names, sections and notes come from the bench (see bench/measure.hpp).
"""

import os
import re
import shutil
import subprocess
import sys

OPEN = re.compile(r'^([ \t]*)--BENCH[ \t]+(\S+)(?:[ \t]+(\S+))?[ \t]*$')
CLOSE = re.compile(r'^[ \t]*BENCH--')
DOC = re.compile(r'/\*\s*@DOC\b.*?\*/', re.S)
VARIANT = re.compile(r'^//\s*variant:\s*(\S+)(.*)$', re.M)
FLAGS = ['-O2', '-DNDEBUG', '-fno-exceptions', '-fno-rtti']


def is_marker(line):
	"""True for a --BENCH or BENCH-- line, which docs leave out."""
	return bool(OPEN.match(line) or CLOSE.match(line))


def blocks(text):
	"""Marker pairs inside @DOC comments: (indent, bench, section, first line, close line), 0-based
	line indices, and errors as (line, message)."""
	starts = [0]
	for m in re.finditer('\n', text):
		starts.append(m.end())
	found, errors = [], []
	for doc in DOC.finditer(text):
		first = text.count('\n', 0, doc.start())
		lines = doc.group(0).split('\n')
		opened = None
		for k, line in enumerate(lines):
			o = OPEN.match(line)
			if o:
				if opened is not None:
					errors.append((first + k, '--BENCH inside another --BENCH block'))
				opened = (o.group(1), o.group(2), o.group(3) or '', first + k)
			elif CLOSE.match(line):
				if opened is None:
					errors.append((first + k, 'BENCH-- without --BENCH'))
				else:
					found.append(opened + (first + k,))
					opened = None
		if opened is not None:
			errors.append((opened[3], '--BENCH without BENCH--'))
	return found, errors


def check(path, text, repo):
	"""Errors of path's markers, as printable lines."""
	found, errors = blocks(text)
	out = [f'{path}:{line + 1}: {message}' for line, message in errors]
	for _, bench, _, line, _ in found:
		if not os.path.isfile(os.path.join(repo, bench)):
			out.append(f'{path}:{line + 1}: --BENCH names a missing file: {bench}')
	return out


def variants(source):
	"""(label, flags) of each build a bench declares besides the baseline."""
	declared = [(m.group(1), m.group(2).split()) for m in VARIANT.finditer(source)]
	return declared or [('ca', [])]


def compiler_label(cxx):
	"""clang 18, gcc 13: family and major version from --version."""
	first = subprocess.run([cxx, '--version'], capture_output=True, text=True, check=True).stdout.split('\n')[0]
	major = re.search(r'(\d+)\.\d+(?:\.\d+)?', first)
	family = 'clang' if 'clang' in first else 'gcc' if 'g++' in first or 'gcc' in first else os.path.basename(cxx)
	return f'{family} {major.group(1)}' if major else family


def cpu_name():
	"""The processor, as /proc/cpuinfo names it, trademarks and clock dropped."""
	try:
		with open('/proc/cpuinfo', encoding='utf-8') as fh:
			for line in fh:
				if line.startswith('model name'):
					name = line.split(':', 1)[1]
					name = re.sub(r'\((R|TM)\)|\bCPU\b|@.*$', '', name)
					return ' '.join(name.split())
	except OSError:
		pass
	return 'unknown processor'


def standard(cxx):
	return 'c++2c' if 'clang' in os.path.basename(cxx) else 'c++23'


def run_command(command, cwd):
	done = subprocess.run(command, cwd=cwd, capture_output=True, text=True)
	if done.returncode != 0:
		raise RuntimeError(f'{" ".join(command)}\n{done.stdout}{done.stderr}')
	return done.stdout


def measure(binary, cwd):
	"""Rows of a bench binary: [(section, case, cycles, note)]."""
	pin = ['taskset', '-c', '2'] if shutil.which('taskset') else []
	rows = []
	for line in run_command(pin + [binary], cwd).split('\n')[1:]:
		if line:
			f = line.split('\t')
			rows.append((f[0], f[1], int(f[2]), f[5] if len(f) > 5 else ''))
	return rows


def results(repo, benches, compilers, log):
	"""For each bench: (columns, rows); columns are labels, rows (section, case, ratios, note)."""
	out = {bench: ([], {}) for bench in benches}
	order = {bench: [] for bench in benches}
	for cxx in compilers:
		label = compiler_label(cxx)
		log(f'bench: building the release library with {cxx}')
		run_command(['make', '--no-print-directory', '-s', f'CXX={cxx}', 'MODE=release'], repo)
		lib = os.path.join('build', f'{os.path.basename(cxx)}-release', 'libca.a')
		for bench in benches:
			name = os.path.splitext(os.path.basename(bench))[0]
			build = os.path.join('build', 'bench', name, os.path.basename(cxx))
			os.makedirs(os.path.join(repo, build), exist_ok=True)
			flags = [cxx, f'-std={standard(cxx)}'] + FLAGS
			with open(os.path.join(repo, bench), encoding='utf-8') as fh:
				builds = variants(fh.read())
			log(f'bench: {bench}, {label}: baseline and {", ".join(v for v, _ in builds)}')
			base = os.path.join(build, 'std')
			run_command(flags + ['-DUSE_STD', bench, '-o', base], repo)
			baseline = {(s, c): (cycles, note) for s, c, cycles, note in measure(os.path.join(repo, base), repo)}
			columns, rows = out[bench]
			for variant, extra in builds:
				binary = os.path.join(build, variant)
				run_command(flags + ['-Isrc/lib'] + extra + [bench, '-o', binary, lib, '-pthread'], repo)
				columns.append(label if len(builds) == 1 else f'{label} {variant}')
				for s, c, cycles, note in measure(os.path.join(repo, binary), repo):
					if (s, c) not in rows:
						rows[(s, c)] = ({}, note or baseline.get((s, c), (0, ''))[1])
						order[bench].append((s, c))
					std = baseline.get((s, c), (0, ''))[0]
					rows[(s, c)][0][columns[-1]] = cycles / std if std else None
	return {bench: (cols, [(s, c, rows[(s, c)][0], rows[(s, c)][1]) for s, c in order[bench]])
		for bench, (cols, rows) in out.items()}


def ratio(value):
	if value is None:
		return '-'
	if value < 0.1:
		return f'{value:.3f}x'
	if value < 10:
		return f'{value:.2f}x'
	return f'{value:.1f}x'


def table(bench, section, columns, rows, where):
	"""The generated lines of one block, without indentation."""
	chosen = [r for r in rows if not section or r[0] == section]
	notes = any(note for _, _, _, note in chosen)
	head = ['Case'] + columns + (['Notes'] if notes else [])
	row = lambda cells: '|' + ''.join(f' {c} |' if c else ' |' for c in cells)
	lines = [f'{bench} on {where}: user cycles relative to the baseline.', '', row(head), '|' + '---|' * len(head)]
	for _, case, ratios, note in chosen:
		lines.append(row([case] + [ratio(ratios.get(c)) for c in columns] + ([note] if notes else [])))
	return lines


def inject(text, measured, where, only):
	"""text with every block of a measured bench regenerated."""
	found, _ = blocks(text)
	lines = text.split('\n')
	for indent, bench, section, first, close in reversed(found):
		if bench not in measured or (only and bench not in only):
			continue
		columns, rows = measured[bench]
		body = [indent + l if l else '' for l in table(bench, section, columns, rows, where)]
		lines[first + 1:close] = body
	return '\n'.join(lines)


def run(repo, lib, names, compilers, log=lambda m: print(m, file=sys.stderr)):
	"""Regenerates the blocks of every header under lib (only benches named in names when
	given); returns the number of files rewritten."""
	files = {}
	for d, dirs, entries in os.walk(lib):
		dirs.sort()
		if os.path.basename(d) == 'ca':
			dirs[:] = []
			continue
		for n in sorted(entries):
			if n.endswith(('.hpp', '.cpp')):
				path = os.path.join(d, n)
				with open(path, encoding='utf-8') as fh:
					text = fh.read()
				if '--BENCH' in text:
					files[path] = text
	wanted = {f'bench/{n}.cpp' if '/' not in n else n for n in names}
	benches = sorted({b for t in files.values() for _, b, _, _, _ in blocks(t)[0] if not wanted or b in wanted})
	if not benches:
		log('bench: no --BENCH block to fill')
		return 0
	measured = results(repo, benches, compilers, log)
	where = f'{cpu_name()}, -O2, min of 9 runs'
	written = 0
	for path, text in files.items():
		updated = inject(text, measured, where, set(benches))
		if updated != text:
			with open(path, 'w', encoding='utf-8') as fh:
				fh.write(updated)
			log(f'bench: wrote {os.path.relpath(path, repo)}')
			written += 1
	return written
