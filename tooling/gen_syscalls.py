#!/usr/bin/env python3
"""Generate the ca::syscall table of the machine it runs on.

Usage: gen_syscalls.py [--dry-run] [--kernel-headers DIR] [--kernel-cc CC] [--clean | --fclean]
       (`make syscalls`)
	--clean leaves nothing to remove but bytecode; --fclean also removes this machine's table and
	rewrites syscalls.hpp from the tables left, or removes it with the last (see cleanup.py).

Writes, under src/lib/3-system/syscall/:
	table/<platform>_<arch>.hpp   ca::syscall::number::<name>, and ca::syscall::<name>(...) taking
	                              exactly the syscall's parameters, named as the kernel names them
	table/<platform>_<arch>.cpp   numbers checked against <sys/syscall.h> at library build
	syscalls.hpp                  includes the table of the build target, from every table present

Each platform has a backend that answers: which syscalls the running kernel implements, with
which numbers, and with which parameters.

linux    The build headers of the running kernel (/lib/modules/$(uname -r)/build, package
         linux-headers-$(uname -r)) are the source of truth:
           names       generated uapi unistd_64.h: name -> number
           entries     generated asm syscalls_64.h: number -> entry point (sys_newfstat for fstat,
                       sys_umount for umount2), sys_ni_syscall for removed ones, NORETURN marks
           parameters  prototypes of include/linux/syscalls.h and asm/syscalls.h, preprocessed
                       with the kernel's own config (autoconf.h) so #ifdef variants resolve as
                       built (clone takes 5 on x86_64); arch syscalls defined only in .c files
                       (mmap, arch_prctl, ... on x86_64) come from Arch.defined_in_c.
         libseccomp (via ctypes) cross-checks every number it knows; a disagreement is an error.
         /proc/kallsyms drops syscalls configured out of this kernel: their entry point is a weak
         alias of sys_ni_syscall (COND_SYSCALL). Unreadable symbols skip that filter, reported.
macos    not implemented. No seccomp; the plan is the SYS_* numbers of <sys/syscall.h> from the
         SDK, parameters from the SDK's syscall prototypes, every syscall taken as available (XNU
         exports no list of its own), and syscall/impl/enter.cpp wired to syscall(2), deprecated
         since macOS 10.12. Apple does not keep raw syscall numbers stable: ca::sys is the
         portable path.
"""

import argparse
import ctypes
import ctypes.util
import os
import platform
import re
import subprocess
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cleanup  # noqa: E402

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(REPO, 'src', 'lib', '3-system', 'syscall')
WIDTH = 78
MAX_NUMBER = 4096
MAX_ARGUMENTS = 6

KEYWORDS = {
	'alignas', 'alignof', 'and', 'and_eq', 'asm', 'auto', 'bitand', 'bitor', 'bool', 'break',
	'case', 'catch', 'char', 'class', 'compl', 'concept', 'const', 'consteval', 'constexpr',
	'constinit', 'const_cast', 'continue', 'co_await', 'co_return', 'co_yield', 'decltype',
	'default', 'delete', 'do', 'double', 'dynamic_cast', 'else', 'enum', 'explicit', 'export',
	'extern', 'false', 'float', 'for', 'friend', 'goto', 'if', 'inline', 'int', 'long', 'mutable',
	'namespace', 'new', 'noexcept', 'not', 'not_eq', 'nullptr', 'operator', 'or', 'or_eq',
	'private', 'protected', 'public', 'register', 'reinterpret_cast', 'requires', 'return',
	'short', 'signed', 'sizeof', 'static', 'static_assert', 'static_cast', 'struct', 'switch',
	'template', 'this', 'thread_local', 'throw', 'true', 'try', 'typedef', 'typeid', 'typename',
	'union', 'unsigned', 'using', 'virtual', 'void', 'volatile', 'wchar_t', 'while', 'xor', 'xor_eq',
}
RESERVED = {'invoke', 'number', 'impl', 'is_argument', 'are_arguments', 'has_table', 'ca'}
TYPE_WORDS = {'unsigned', 'signed', 'long', 'int', 'short', 'char', 'void', 'const', 'volatile'}


class Unsupported(Exception):
	pass


class Syscall:

	def __init__(self, name, number, entry, parameters, noreturn):
		self.name = name
		self.number = number
		self.entry = entry
		self.parameters = parameters
		self.noreturn = noreturn


class Table:
	"""Syscalls of one platform and architecture, plus what the report needs."""

	def __init__(self, platform_name, arch, origin):
		self.platform = platform_name
		self.arch = arch
		self.origin = origin
		self.syscalls = []
		self.notes = []
		self.dropped = {}

	@property
	def key(self):
		return f'{self.platform}_{self.arch}'


# == linux =================================================================

class Arch:

	def __init__(self, token, kallsyms, kernel_arch, names, entries, defined_in_c):
		self.token = token
		self.kallsyms = kallsyms
		self.kernel_arch = kernel_arch
		self.names = names
		self.entries = entries
		self.defined_in_c = defined_in_c


class LinuxBackend:

	# To add an arch: its libseccomp SCMP_ARCH token, the kallsyms prefix of its syscall entry
	# points, its directory under arch/, the generated name and entry tables of its build
	# headers, and the parameters of the arch syscalls only its .c files declare.
	ARCHES = {
		'x86_64': Arch(
			token=0xC000003E,
			kallsyms='__x64_sys_',
			kernel_arch='x86',
			names='arch/x86/include/generated/uapi/asm/unistd_64.h',
			entries='arch/x86/include/generated/asm/syscalls_64.h',
			defined_in_c={
				'sys_mmap': 'unsigned long addr, unsigned long len, unsigned long prot, unsigned long flags, unsigned long fd, unsigned long off',
				'sys_arch_prctl': 'int option, unsigned long arg2',
				'sys_iopl': 'unsigned int level',
				'sys_modify_ldt': 'int func, void *ptr, unsigned long bytecount',
				'sys_rt_sigreturn': 'void',
			}),
	}

	def __init__(self, opts):
		self.opts = opts

	def collect(self):
		arch_name = platform.machine()
		if arch_name not in self.ARCHES:
			raise Unsupported(f'linux {arch_name}: add it to LinuxBackend.ARCHES')
		arch = self.ARCHES[arch_name]
		release = platform.release()
		build = self.opts.kernel_headers or f'/lib/modules/{release}/build'
		if not os.path.isfile(os.path.join(build, arch.entries)):
			raise SystemExit(f'gen_syscalls: no kernel build headers at {build}; install linux-headers-{release}')
		table = Table('linux', arch_name, f'kernel {release}')

		names = self._names(os.path.join(build, arch.names))
		entries = self._entries(os.path.join(build, arch.entries))
		prototypes = self._prototypes(build, arch)
		missing = sorted({entries[n][0] for n in names.values() if n in entries} - set(prototypes) - set(arch.defined_in_c) - {'sys_ni_syscall'})
		if missing:
			raise SystemExit(f'gen_syscalls: no prototype for {" ".join(missing)}; add them to Arch.defined_in_c')
		prototypes.update(arch.defined_in_c)
		table.notes.append(f'kernel headers: {len(names)} names, {len(entries)} entries, {len(prototypes)} prototypes')

		strong, weak = self._kernel_entries(arch.kallsyms)
		if not strong:
			table.notes.append('kernel symbols unreadable: configured-out syscalls not filtered')
		for name, number in sorted(names.items(), key=lambda item: item[1]):
			if number not in entries:
				table.dropped.setdefault('not in the entry table', []).append(name)
				continue
			entry, noreturn = entries[number]
			if entry == 'sys_ni_syscall':
				table.dropped.setdefault('removed (sys_ni_syscall)', []).append(name)
				continue
			short = entry[len('sys_'):]
			if strong and short not in strong:
				reason = 'configured out (weak symbol)' if short in weak else 'no entry point in kallsyms'
				table.dropped.setdefault(reason, []).append(name)
				continue
			parameters = self._parameters(prototypes[entry])
			if len(parameters) > MAX_ARGUMENTS:
				raise SystemExit(f'gen_syscalls: {name} takes {len(parameters)} arguments')
			table.syscalls.append(Syscall(name, number, entry, parameters, noreturn))

		self._cross_check(table, arch, names)
		return table

	def _names(self, path):
		with open(path) as f:
			found = re.findall(r'^#define __NR_(\w+) (\d+)$', f.read(), re.M)
		return {name: int(value) for name, value in found if name != 'syscalls'}

	def _entries(self, path):
		with open(path) as f:
			text = f.read()
		return {int(number): (entry, kind == '_NORETURN')
			for kind, number, entry in re.findall(r'^__SYSCALL(\w*)\((\d+), (\w+)\)$', text, re.M)}

	def _prototypes(self, build, arch):
		k = arch.kernel_arch
		includes = [
			f'arch/{k}/include', f'arch/{k}/include/generated', 'include', f'arch/{k}/include/uapi',
			f'arch/{k}/include/generated/uapi', 'include/uapi', 'include/generated/uapi',
		]
		forced = ['include/linux/compiler-version.h', 'include/linux/kconfig.h', 'include/linux/compiler_types.h']
		gcc_include = subprocess.run([self.opts.kernel_cc, '-print-file-name=include'], capture_output=True, text=True).stdout.strip()
		command = [self.opts.kernel_cc, '-isystem', gcc_include] + [f'-I{os.path.join(build, i)}' for i in includes] \
			+ [a for f in forced for a in ('-include', os.path.join(build, f))] \
			+ ['-E', '-P', '-D__KERNEL__', '-DCC_USING_FENTRY', '-nostdinc', '-x', 'c', '-']
		source = '#undef CONFIG_ARCH_HAS_SYSCALL_WRAPPER\n#include <linux/syscalls.h>\n#include <asm/syscalls.h>\n'
		done = subprocess.run(command, input=source, capture_output=True, text=True)
		if done.returncode != 0:
			raise SystemExit(f'gen_syscalls: preprocessing the kernel prototypes failed:\n{done.stderr[-2000:]}')
		flat = ' '.join(done.stdout.split())
		return {entry: params for entry, params in re.findall(r'\blong (sys_\w+)\(([^()]*)\);', flat)}

	def _parameters(self, text):
		text = ' '.join(text.split())
		if text in ('', 'void'):
			return []
		parameters = []
		for raw in text.split(','):
			raw = raw.strip()
			tokens = re.findall(r'[A-Za-z_]\w*|\*', raw)
			name = None
			if len(tokens) >= 2 and re.fullmatch(r'[A-Za-z_]\w*', tokens[-1]) and tokens[-1] not in TYPE_WORDS \
				and tokens[-2] not in ('struct', 'union', 'enum'):
				name = tokens[-1]
			parameters.append((raw, name))
		return parameters

	def _kernel_entries(self, prefix):
		strong, weak = set(), set()
		try:
			with open('/proc/kallsyms') as f:
				for line in f:
					parts = line.split()
					if len(parts) < 3 or not parts[2].startswith(prefix):
						continue
					(weak if parts[1] in 'Ww' else strong).add(parts[2][len(prefix):])
		except OSError:
			pass
		return strong, weak

	def _cross_check(self, table, arch, names):
		path = ctypes.util.find_library('seccomp') or 'libseccomp.so.2'
		try:
			lib = ctypes.CDLL(path)
		except OSError:
			table.notes.append('libseccomp not found: numbers not cross-checked')
			return
		libc = ctypes.CDLL(None)
		libc.free.argtypes = [ctypes.c_void_p]
		lib.seccomp_arch_native.restype = ctypes.c_uint32
		lib.seccomp_syscall_resolve_num_arch.argtypes = [ctypes.c_uint32, ctypes.c_int]
		lib.seccomp_syscall_resolve_num_arch.restype = ctypes.c_void_p

		class Version(ctypes.Structure):
			_fields_ = [('major', ctypes.c_uint), ('minor', ctypes.c_uint), ('micro', ctypes.c_uint)]

		lib.seccomp_version.restype = ctypes.POINTER(Version)
		v = lib.seccomp_version().contents
		native = lib.seccomp_arch_native()
		if native != arch.token:
			raise SystemExit(f'gen_syscalls: libseccomp native arch {native:#x}, expected {arch.token:#x}')
		known = {}
		for number in range(MAX_NUMBER):
			raw = lib.seccomp_syscall_resolve_num_arch(native, number)
			if raw:
				known[ctypes.string_at(raw).decode()] = number
				libc.free(raw)
		for name, number in known.items():
			if name in names and names[name] != number:
				raise SystemExit(f'gen_syscalls: {name} is {number} in libseccomp but {names[name]} in the kernel')
		agreed = sum(1 for name in known if name in names)
		newer = sorted(set(names) - set(known))
		table.notes.append(f'libseccomp {v.major}.{v.minor}.{v.micro}: agrees on {agreed} numbers, lacks {len(newer)} newer ones')


# == macos =================================================================

class MacosBackend:

	def __init__(self, opts):
		self.opts = opts

	def collect(self):
		raise Unsupported('macos: not implemented yet; see the module docstring for the plan')


BACKENDS = {'Linux': LinuxBackend, 'Darwin': MacosBackend}


# == emit ==================================================================

def banner(tabs, title):
	head = '\t' * tabs + f'// -- {title} '
	return head + '-' * (WIDTH - len(head.expandtabs(4)))


def identifier(name):
	if not re.fullmatch(r'[A-Za-z_][A-Za-z0-9_]*', name):
		raise SystemExit(f'gen_syscalls: {name!r} is not an identifier')
	return name + '_' if name in KEYWORDS or name in RESERVED else name


def parameter_names(syscall):
	names = []
	for i, (_, name) in enumerate(syscall.parameters):
		ident = identifier(name) if name else f'arg{i}'
		names.append(ident if ident not in names else f'arg{i}')
	return names


def condition(key):
	plat, arch = key.split('_', 1)
	return f'ca_platform({plat}) && ca_architecture({arch})'


def emit_binding(syscall):
	ident = identifier(syscall.name)
	names = parameter_names(syscall)
	prototype = ', '.join(raw for raw, _ in syscall.parameters) or 'void'
	entry = '' if syscall.entry == f'sys_{syscall.name}' else f' (entry {syscall.entry})'
	params = ', '.join(f'const ca::syscall::is_argument auto {n}' for n in names) or 'void'
	words = ''.join(f', ca::syscall::impl::word({n})' for n in names)
	args = ''.join(f', {n}' for n in names)
	doc = f'\t\t/* @DOC {syscall.name}({prototype}): syscall {syscall.number}{entry}{", never returns" if syscall.noreturn else ""}. */'
	if syscall.noreturn:
		return [
			doc,
			f'\t\t[[noreturn]] inline auto {ident}({params}) noexcept -> void {{', '',
			f'\t\t\tstatic_cast<void>(ca::syscall::impl::enter(ca::syscall::number::{ident}{words}));',
			'\t\t\tca::unreachable();',
			'\t\t}', '',
		]
	return [
		doc,
		f'\t\t[[nodiscard]] inline auto {ident}({params}) noexcept -> ca::result<ca::isz> {{',
		f'\t\t\treturn ca::syscall::invoke(ca::syscall::number::{ident}{args});',
		'\t\t}', '',
	]


def emit_header(table):
	guard = f'ca_system_syscall_table_{table.key}_hpp'
	width = max(len(identifier(s.name)) for s in table.syscalls)
	lines = [
		f'#ifndef {guard}', f'#define {guard}', '',
		'#include "1-primitives/debug/unreachable.hpp"',
		'#include "1-primitives/integers/isz.hpp"',
		'#include "2-vocabulary/result.hpp"',
		'#include "3-system/syscall/argument.hpp"',
		'#include "3-system/syscall/impl/enter.hpp"',
		'#include "3-system/syscall/invoke.hpp"', '',
		f'// Generated by tooling/gen_syscalls.py on {table.platform} {table.arch}, {table.origin}. Do not edit.',
		'', '',
		banner(0, 'CA NAMESPACE'), '', 'namespace ca {', '', '',
		banner(1, 'SYSCALL'), '', '\tnamespace syscall {', '', '',
		banner(2, 'NUMBER'), '',
		f'\t\t/* @DOC Syscall numbers of {table.platform} {table.arch} implemented by {table.origin}. */',
		'\t\tnamespace number {', '',
	]
	for syscall in table.syscalls:
		lines.append(f'\t\t\tinline constexpr ca::isz {identifier(syscall.name).ljust(width)} = {syscall.number};')
	lines += ['', '\t\t} // namespace number', '', '', banner(2, 'CALLS'), '']
	for syscall in table.syscalls:
		lines += emit_binding(syscall)
	lines += ['\t} // namespace syscall', '', '} // namespace ca', '', f'#endif // {guard}', '']
	return '\n'.join(lines)


def emit_source(table):
	lines = [
		'#include "1-primitives/config/architecture.hpp"',
		'#include "1-primitives/config/platform.hpp"',
		f'#include "3-system/syscall/table/{table.key}.hpp"', '',
		f'#if {condition(table.key)}',
		'#	include <sys/syscall.h>', '',
	]
	for syscall in table.syscalls:
		ident = identifier(syscall.name)
		lines += [f'#	if defined(SYS_{syscall.name})', f'static_assert(ca::syscall::number::{ident} == SYS_{syscall.name});', '#	endif']
	lines += ['', '#endif', '']
	return '\n'.join(lines)


def emit_dispatcher(keys):
	guard = 'ca_system_syscall_syscalls_hpp'
	lines = [
		f'#ifndef {guard}', f'#define {guard}', '',
		'#include "1-primitives/config/architecture.hpp"',
		'#include "1-primitives/config/platform.hpp"',
		'#include "3-system/syscall/invoke.hpp"', '',
		'// Generated by tooling/gen_syscalls.py from the tables in table/. Do not edit.', '',
	]
	for i, key in enumerate(keys):
		lines += [f'#{"if" if i == 0 else "elif"} {condition(key)}', f'#	include "3-system/syscall/table/{key}.hpp"']
	if keys:
		lines.append('#endif')
	any_table = ' || '.join(f'({condition(key)})' for key in keys) or '0'
	lines += [
		'', '',
		banner(0, 'CA NAMESPACE'), '', 'namespace ca {', '', '',
		banner(1, 'SYSCALL'), '', '\tnamespace syscall {', '', '',
		banner(2, 'HAS TABLE'), '',
		'\t\t/* @DOC Whether the target has a generated table: number::<name> and <name>(args...). Without',
		'\t\t   one only invoke exists. */',
		f'\t\tinline constexpr bool has_table = {any_table};', '',
		'\t} // namespace syscall', '', '} // namespace ca', '', f'#endif // {guard}', '',
	]
	return '\n'.join(lines)


def report(table):
	print(f'gen_syscalls: {table.platform} {table.arch}, {table.origin}', file=sys.stderr)
	for note in table.notes:
		print(f'  {note}', file=sys.stderr)
	counts = [0] * (MAX_ARGUMENTS + 1)
	for syscall in table.syscalls:
		counts[len(syscall.parameters)] += 1
	spread = ', '.join(f'{n}: {c}' for n, c in enumerate(counts))
	print(f'  generated: {len(table.syscalls)} syscalls, by argument count {spread}', file=sys.stderr)
	for reason, names in table.dropped.items():
		print(f'  dropped, {reason} ({len(names)}): {" ".join(names)}', file=sys.stderr)


def write(path, text, dry_run):
	rel = os.path.relpath(path, REPO)
	if dry_run:
		print(f'  would write {rel}', file=sys.stderr)
		return
	os.makedirs(os.path.dirname(path), exist_ok=True)
	old = open(path).read() if os.path.exists(path) else None
	if old == text:
		print(f'  unchanged {rel}', file=sys.stderr)
		return
	with open(path, 'w') as f:
		f.write(text)
	print(f'  wrote {rel}', file=sys.stderr)


HOST_PLATFORMS = {'Linux': 'linux', 'Darwin': 'macos'}


def table_keys(tables):
	return {f[:-4] for f in os.listdir(tables) if f.endswith('.hpp')} if os.path.isdir(tables) else set()


def fclean_outputs():
	tables = os.path.join(OUT, 'table')
	key = f'{HOST_PLATFORMS.get(platform.system(), platform.system().lower())}_{platform.machine()}'
	paths = [os.path.join(tables, f'{key}.{ext}') for ext in ('hpp', 'cpp')]
	dispatcher = os.path.join(OUT, 'syscalls.hpp')
	left = sorted(table_keys(tables) - {key})
	if not left:
		return paths + [dispatcher]
	write(dispatcher, emit_dispatcher(left), False)
	return paths


def main():
	ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
	ap.add_argument('--dry-run', action='store_true', help='report only, write nothing')
	ap.add_argument('--kernel-headers', help='kernel build headers (default /lib/modules/$(uname -r)/build)')
	ap.add_argument('--kernel-cc', default='gcc', help='compiler to preprocess kernel headers with (the kernel\'s)')
	cleanup.add_arguments(ap)
	opts = ap.parse_args()
	if opts.clean or opts.fclean:
		return cleanup.run(opts, 'gen_syscalls', cleanup.bytecode(), fclean_outputs, [os.path.join(OUT, 'table')])

	system = platform.system()
	if system not in BACKENDS:
		raise SystemExit(f'gen_syscalls: no backend for {system}')
	try:
		table = BACKENDS[system](opts).collect()
	except Unsupported as error:
		raise SystemExit(f'gen_syscalls: {error}')
	report(table)

	tables = os.path.join(OUT, 'table')
	write(os.path.join(tables, f'{table.key}.hpp'), emit_header(table), opts.dry_run)
	write(os.path.join(tables, f'{table.key}.cpp'), emit_source(table), opts.dry_run)
	keys = {table.key} | table_keys(tables)
	write(os.path.join(OUT, 'syscalls.hpp'), emit_dispatcher(sorted(keys)), opts.dry_run)


if __name__ == '__main__':
	sys.exit(main())
