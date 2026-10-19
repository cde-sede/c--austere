#!/usr/bin/env python3
"""Compile every c++ example of the .@DOC files, and run those defining main.

Usage: check_examples.py [FOLDER...] [--cxx CXX] [--mode debug|release]
FOLDER... limits it to the folders under src/lib whose path starts so (2-vocabulary, 5-collections/map).

Each example is a complete translation unit, built with the flags of the Makefile against
build/<cxx>-<mode>/libca.a (make it first) in build/<cxx>-<mode>/examples/, where it also runs:
it must exit 0. Blocks fenced as another language (```text) are skipped. Exits 1 when one fails.
"""

import argparse
import os
import subprocess
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import autodoc  # noqa: E402

LIB_ROOT = os.path.join(autodoc.REPO, 'src', 'lib')


def flags_of(cxx, mode):
	std = 'c++2c' if 'clang' in cxx else 'c++23'
	compile_flags = [f'-std={std}', '-nostdinc++', '-fno-exceptions', '-fno-rtti', '-Wall', '-Wextra', '-Wpedantic',
		'-Werror', '-pthread', '-I', LIB_ROOT] + (['-O2', '-DNDEBUG'] if mode == 'release' else ['-g', '-O1'])
	link_flags = ['-nostdinc++', '-pthread'] + (['-nostdlib++'] if 'clang' in cxx else [])
	return compile_flags, link_flags


def examples(only):
	for d, dirs, names in os.walk(LIB_ROOT):
		dirs.sort()
		if autodoc.FOLDER_DOC not in names:
			continue
		rel = os.path.relpath(d, LIB_ROOT)
		if only and not any(rel.startswith(o) for o in only):
			continue
		with open(os.path.join(d, autodoc.FOLDER_DOC), encoding='utf-8') as fh:
			blocks = [b[2] for b in autodoc.folder_blocks(fh.read()) if b[0] == 'code' and b[1] == 'c++']
		for n, code in enumerate(blocks):
			yield rel, n, code


def check(rel, n, code, out, cxx, compile_flags, link_flags, lib):
	src = os.path.join(out, f'{rel.replace("/", "_")}_{n}.cpp')
	with open(src, 'w', encoding='utf-8') as fh:
		fh.write(code + '\n')
	runs = 'main(' in code
	exe = src[:-len('.cpp')]
	cmd = [cxx] + compile_flags + ([src, '-o', exe, lib] + link_flags if runs else ['-fsyntax-only', src])
	built = subprocess.run(cmd, capture_output=True, text=True)
	if built.returncode:
		return f'compile failed\n{built.stderr[:3000]}'
	if not runs:
		return None
	ran = subprocess.run([exe], capture_output=True, text=True, timeout=20, cwd=out)
	if ran.returncode:
		return f'exit {ran.returncode}\n{ran.stdout[-1500:]}{ran.stderr[-1500:]}'
	return None


def main(argv=None):
	ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
	ap.add_argument('folders', nargs='*', metavar='FOLDER')
	ap.add_argument('--cxx', default='clang++')
	ap.add_argument('--mode', choices=('debug', 'release'), default='debug')
	opts = ap.parse_args(argv)
	build = os.path.join(autodoc.REPO, 'build', f'{os.path.basename(opts.cxx)}-{opts.mode}')
	lib = os.path.join(build, 'libca.a')
	if not os.path.isfile(lib):
		print(f'check_examples: {lib} missing: make CXX={opts.cxx} MODE={opts.mode} first', file=sys.stderr)
		return 1
	out = os.path.join(build, 'examples')
	os.makedirs(out, exist_ok=True)
	compile_flags, link_flags = flags_of(opts.cxx, opts.mode)
	count = bad = 0
	for rel, n, code in examples(opts.folders):
		count += 1
		failure = check(rel, n, code, out, opts.cxx, compile_flags, link_flags, lib)
		if failure:
			bad += 1
			print(f'{os.path.join(LIB_ROOT, rel, autodoc.FOLDER_DOC)}: example {n}: {failure}', file=sys.stderr)
	print(f'check_examples: {count - bad}/{count} examples pass ({opts.cxx}, {opts.mode})', file=sys.stderr)
	return 1 if bad else 0


if __name__ == '__main__':
	sys.exit(main())
