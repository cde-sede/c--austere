"""--clean and --fclean for the generators, as make's clean and fclean.

	--clean    remove the scratch a generator leaves: downloads, caches, Python bytecode
	--fclean   --clean, and remove every file the generator wrote

Only paths the generator owns are removed; directories it created are removed once empty.
Running the generator again restores everything. Generators set sys.dont_write_bytecode
before importing tooling modules, so a run leaves no __pycache__; --clean removes old ones.
"""

import glob
import os
import shutil
import sys

TOOLING = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(TOOLING)


def add_arguments(ap):
	group = ap.add_mutually_exclusive_group()
	group.add_argument('--clean', action='store_true', help='remove scratch: downloads, caches, bytecode')
	group.add_argument('--fclean', action='store_true', help='--clean, and remove every generated file')


def bytecode(*modules):
	"""Bytecode of modules, of cleanup and of the running generator itself."""
	cache = os.path.join(TOOLING, '__pycache__')
	script = os.path.splitext(os.path.basename(sys.argv[0]))[0]
	return [p for m in modules + ('cleanup', script) for p in glob.glob(os.path.join(cache, f'{m}.*.pyc'))]


def remove(name, paths, dirs=()):
	removed = 0
	for path in paths:
		if os.path.isdir(path) and not os.path.islink(path):
			shutil.rmtree(path)
		elif os.path.lexists(path):
			os.remove(path)
		else:
			continue
		removed += 1
		print(f'  removed {os.path.relpath(path, REPO)}', file=sys.stderr)
	for d in sorted(set(dirs) | {os.path.join(TOOLING, '__pycache__')}, key=len, reverse=True):
		if os.path.isdir(d) and not os.listdir(d):
			os.rmdir(d)
			print(f'  removed {os.path.relpath(d, REPO)}/', file=sys.stderr)
	print(f'{name}: {removed} removed' if removed else f'{name}: clean', file=sys.stderr)
	return 0


def run(opts, name, scratch, outputs=(), dirs=()):
	"""outputs: the generated files (list, or a callable returning one, called on --fclean only)."""
	paths = list(scratch)
	if opts.fclean:
		paths += list(outputs() if callable(outputs) else outputs)
	return remove(name, paths, dirs if opts.fclean else ())
