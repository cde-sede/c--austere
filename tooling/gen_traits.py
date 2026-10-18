#!/usr/bin/env python3
"""Generate lib/1-primitives/traits/ (std <type_traits> parity) from the specs below.

Usage: gen_traits.py [lib] [--clean | --fclean]   (default: <repo>/src/lib)

Source of truth for traits/: edit the specs here and regenerate, never the headers.
Writes traits/*.hpp and traits/impl/*.hpp only; includes are derived from ca:: references.
--clean leaves nothing to remove but bytecode; --fclean also removes every header of the specs
and traits/ once empty (see cleanup.py).
"""

import argparse
import os
import re
import sys

sys.dont_write_bytecode = True
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import cleanup  # noqa: E402

AP = argparse.ArgumentParser(description=__doc__.split('\n')[0])
AP.add_argument('lib', nargs='?', default=os.path.join(cleanup.REPO, 'src', 'lib'))
cleanup.add_arguments(AP)
OPTS = AP.parse_args()
LIB = OPTS.lib
OUT = os.path.join(LIB, '1-primitives')

SPECS = {}
EXTRA_SYS = {}


def split_top(s):
	out, depth, cur = [], 0, ''
	for ch in s:
		if ch in '<([':
			depth += 1
		elif ch in '>)]':
			depth -= 1
		if ch == ',' and depth == 0:
			out.append(cur)
			cur = ''
		else:
			cur += ch
	if cur.strip():
		out.append(cur)
	return out


def args_of(tp):
	res = []
	for p in split_top(tp):
		p = p.strip()
		name = p.split()[-1]
		res.append(name + '...' if '...' in p else name)
	return ', '.join(res)


def struct(name, tp, spec=None, members=(), req=None, level=1):
	i = '    ' * level
	m = '    ' * (level + 2)
	args = spec if spec is not None else args_of(tp)
	lines = [f'{i}template <{tp}>']
	if req:
		lines.append(f'{i}requires {req}')
	lines.append(f'{i}struct {name}{"<" + spec + ">" if spec is not None else ""} final {{')
	lines.append(f'{m}using self = ca::impl::{name}<{args}>;')
	for x in members:
		lines += [m + l if l else '' for l in x.split('\n')]
	lines.append(f'{m}{name}(void) = delete;')
	lines.append(f'{i}}}; // struct {name}')
	return '\n'.join(lines)


def impl(*blocks):
	return 'namespace impl {\n\n' + '\n\n'.join(blocks) + '\n\n} // namespace impl'


def concept(name, expr, tp='typename T', mute=None):
	c = f' // {mute}: ignore' if mute else ''
	return f'template <{tp}>{c}\nconcept {name} = {expr};'


def alias(name, expr, tp='typename T', req=None):
	r = f'requires {req}\n' if req else ''
	return f'template <{tp}>\n{r}using {name} = {expr};'


def var(name, expr, typ='bool', tp='typename T', spec='', level=1):
	i = '    ' * level
	return f'{i}template <{tp}>\n{i}inline constexpr {typ} {name}{spec} = {expr};'


def add(name, *parts):
	SPECS[name] = '\n\n\n'.join(parts)


def builtin(name, expr, tp='typename T', mute=None):
	add(name, concept(name, expr, tp, mute))


def type_struct(name, primary, specs, tp='typename T', req_alias=None):
	blocks = [struct(name, tp, members=[f'using type = {primary};'] if primary else [])]
	for stp, spec, typ, *req in specs:
		blocks.append(struct(name, stp, spec, [f'using type = {typ};'], req[0] if req else None))
	add(name, impl(*blocks), alias(name, f'typename ca::impl::{name}<{args_of(tp)}>::type', tp, req_alias))


def value_spec(name, specs, primary='false', typ='bool'):
	blocks = [var(name, primary)]
	for stp, spec, val in specs:
		blocks.append(var(name, val, typ, stp, f'<{spec}>'))
	return impl(*blocks)


# -- helper classes ----------------------------------------------------------

def banner(title, indent):
	head = '\t' * indent + '// -- ' + title + ' '
	return head + '-' * (78 - (len(head) - indent + 4 * indent))


SPECS['integral_constant'] = '\n'.join([
	'template <typename T, T V> // final: ignore',
	'struct integral_constant {',
	'',
	'',
	'    private:',
	'',
	'@@private types',
	'',
	'        using self = ca::integral_constant<T, V>;',
	'',
	'',
	'    public:',
	'',
	'@@public types',
	'',
	'        using value_type = T;',
	'        using type       = self;',
	'',
	'',
	'@@public constants',
	'',
	'        static constexpr T value = V;',
	'',
	'',
	'@@accessors',
	'',
	'        constexpr operator value_type(void) const noexcept {',
	'            return value;',
	'        }',
	'',
	'        constexpr auto operator()(void) const noexcept -> value_type {',
	'            return value;',
	'        }',
	'',
	'}; // struct integral_constant',
])
add('bool_constant', alias('bool_constant', 'ca::integral_constant<bool, B>', 'bool B'))
add('true_type', 'using true_type = ca::bool_constant<true>;')
add('false_type', 'using false_type = ca::bool_constant<false>;')


# -- primary type categories -------------------------------------------------

add('is_void', concept('is_void', 'ca::is_same<ca::remove_cv<T>, void>'))
add('is_null_pointer', concept('is_null_pointer', 'ca::is_same<ca::remove_cv<T>, decltype(nullptr)>'))
add('is_integral', concept('is_integral', 'ca::impl::is_any_of<ca::remove_cv<T>,\n'
	'    bool, char, signed char, unsigned char, wchar_t, char8_t, char16_t, char32_t,\n'
	'    short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>'))
add('is_floating_point', concept('is_floating_point', 'ca::impl::is_any_of<ca::remove_cv<T>, float, double, long double>'))
add('is_array', concept('is_array', 'ca::is_bounded_array<T> || ca::is_unbounded_array<T>'))
builtin('is_enum', '__is_enum(T)')
builtin('is_union', '__is_union(T)')
builtin('is_class', '__is_class(T)')
add('is_function', concept('is_function', '!ca::is_const<const T> && !ca::is_reference<T>'))
add('is_pointer', value_spec('is_pointer', [('typename T', 'T*', 'true')]),
	concept('is_pointer', 'ca::impl::is_pointer<ca::remove_cv<T>>'))
add('is_lvalue_reference', value_spec('is_lvalue_reference', [('typename T', 'T&', 'true')]),
	concept('is_lvalue_reference', 'ca::impl::is_lvalue_reference<T>'))
add('is_rvalue_reference', value_spec('is_rvalue_reference', [('typename T', 'T&&', 'true')]),
	concept('is_rvalue_reference', 'ca::impl::is_rvalue_reference<T>'))
add('is_member_function_pointer',
	value_spec('is_member_function_pointer', [('typename T, typename C', 'T C::*', 'ca::is_function<T>')]),
	concept('is_member_function_pointer', 'ca::impl::is_member_function_pointer<ca::remove_cv<T>>'))
add('is_member_object_pointer',
	concept('is_member_object_pointer', 'ca::is_member_pointer<T> && !ca::is_member_function_pointer<T>'))


# -- composite type categories -----------------------------------------------

add('is_fundamental', concept('is_fundamental', 'ca::is_arithmetic<T> || ca::is_void<T> || ca::is_null_pointer<T>'))
add('is_arithmetic', concept('is_arithmetic', 'ca::is_integral<T> || ca::is_floating_point<T>'))
add('is_scalar', concept('is_scalar', 'ca::is_arithmetic<T> || ca::is_enum<T> || ca::is_pointer<T>\n'
	'    || ca::is_member_pointer<T> || ca::is_null_pointer<T>'))
add('is_object', concept('is_object', '!ca::is_function<T> && !ca::is_reference<T> && !ca::is_void<T>'))
add('is_compound', concept('is_compound', '!ca::is_fundamental<T>'))
add('is_reference', concept('is_reference', 'ca::is_lvalue_reference<T> || ca::is_rvalue_reference<T>'))
add('is_member_pointer', value_spec('is_member_pointer', [('typename T, typename C', 'T C::*', 'true')]),
	concept('is_member_pointer', 'ca::impl::is_member_pointer<ca::remove_cv<T>>'))


# -- type properties ---------------------------------------------------------

add('is_const', value_spec('is_const', [('typename T', 'const T', 'true')]), concept('is_const', 'ca::impl::is_const<T>'))
add('is_volatile', value_spec('is_volatile', [('typename T', 'volatile T', 'true')]),
	concept('is_volatile', 'ca::impl::is_volatile<T>'))
builtin('is_trivial', '__is_trivial(T)')
builtin('is_trivially_copyable', '__is_trivially_copyable(T)')
builtin('is_standard_layout', '__is_standard_layout(T)')
builtin('is_pod', '__is_pod(T)')
builtin('is_empty', '__is_empty(T)')
builtin('is_polymorphic', '__is_polymorphic(T)')
builtin('is_abstract', '__is_abstract(T)')
builtin('is_final', '__is_final(T)')
add('is_aggregate', concept('is_aggregate', '__is_aggregate(ca::remove_cv<T>)'))
add('is_implicit_lifetime', '\n'.join([
	'#if ca_has_builtin(__builtin_is_implicit_lifetime)',
	concept('is_implicit_lifetime', '__builtin_is_implicit_lifetime(T)'),
	'#else',
	concept('is_implicit_lifetime', 'ca::is_scalar<T> || ca::is_array<T>\n'
		'    || ((ca::is_class<T> || ca::is_union<T>) && ca::is_trivially_destructible<T>\n'
		'    && (ca::is_aggregate<T> || ca::is_trivially_default_constructible<T>\n'
		'    || ca::is_trivially_copy_constructible<T> || ca::is_trivially_move_constructible<T>))'),
	'#endif',
]))
add('is_signed', concept('is_signed', 'ca::is_arithmetic<T> && (static_cast<T>(-1) < static_cast<T>(0))'))
add('is_unsigned', concept('is_unsigned', 'ca::is_arithmetic<T> && (static_cast<T>(0) < static_cast<T>(-1))'))
add('is_bounded_array', value_spec('is_bounded_array', [('typename T, ca::usz N', 'T[N]', 'true')]),
	concept('is_bounded_array', 'ca::impl::is_bounded_array<T>'))
add('is_unbounded_array', value_spec('is_unbounded_array', [('typename T', 'T[]', 'true')]),
	concept('is_unbounded_array', 'ca::impl::is_unbounded_array<T>'))
add('is_scoped_enum', concept('is_scoped_enum', 'ca::is_enum<T> && !ca::is_convertible<T, ca::underlying_type<T>>'))
add('has_unique_object_representations', concept('has_unique_object_representations',
	'__has_unique_object_representations(ca::remove_cv<ca::remove_all_extents<T>>)', mute='concept-name'))


# -- supported operations ----------------------------------------------------

TA = 'typename T, typename... Args'
builtin('is_constructible', '__is_constructible(T, Args...)', TA)
builtin('is_trivially_constructible', '__is_trivially_constructible(T, Args...)', TA)
builtin('is_nothrow_constructible', '__is_nothrow_constructible(T, Args...)', TA)
for k, base in (('', 'is_constructible'), ('trivially_', 'is_trivially_constructible'),
		('nothrow_', 'is_nothrow_constructible')):
	add(f'is_{k}default_constructible', concept(f'is_{k}default_constructible', f'ca::{base}<T>'))
	add(f'is_{k}copy_constructible',
		concept(f'is_{k}copy_constructible', f'ca::{base}<T, ca::add_lvalue_reference<const T>>'))
	add(f'is_{k}move_constructible',
		concept(f'is_{k}move_constructible', f'ca::{base}<T, ca::add_rvalue_reference<T>>'))
TU = 'typename T, typename U'
builtin('is_assignable', '__is_assignable(T, U)', TU)
builtin('is_trivially_assignable', '__is_trivially_assignable(T, U)', TU)
builtin('is_nothrow_assignable', '__is_nothrow_assignable(T, U)', TU)
for k, base in (('', 'is_assignable'), ('trivially_', 'is_trivially_assignable'),
		('nothrow_', 'is_nothrow_assignable')):
	add(f'is_{k}copy_assignable', concept(f'is_{k}copy_assignable',
		f'ca::{base}<ca::add_lvalue_reference<T>, ca::add_lvalue_reference<const T>>'))
	add(f'is_{k}move_assignable', concept(f'is_{k}move_assignable',
		f'ca::{base}<ca::add_lvalue_reference<T>, ca::add_rvalue_reference<T>>'))

add('is_destructible', impl(
	'    template <typename T>\n'
	'    concept is_destructible_object = requires(T& t) {\n'
	'        t.~T();\n'
	'    };'),
	concept('is_destructible', 'ca::is_reference<T>\n'
		'    || (!ca::is_void<T> && !ca::is_function<T> && !ca::is_unbounded_array<T>\n'
		'    && ca::impl::is_destructible_object<ca::remove_all_extents<T>>)'))
add('is_nothrow_destructible', impl(
	'    template <typename T>\n'
	'    concept is_nothrow_destructible_object = requires(T& t) {\n'
	'        { t.~T() } noexcept;\n'
	'    };'),
	concept('is_nothrow_destructible', 'ca::is_reference<T>\n'
		'    || (ca::is_destructible<T> && ca::impl::is_nothrow_destructible_object<ca::remove_all_extents<T>>)'))
add('is_trivially_destructible', '\n'.join([
	'#if ca_has_builtin(__is_trivially_destructible)',
	concept('is_trivially_destructible', '__is_trivially_destructible(T)'),
	'#else',
	concept('is_trivially_destructible', 'ca::is_destructible<T> && __has_trivial_destructor(T)'),
	'#endif',
]))
builtin('has_virtual_destructor', '__has_virtual_destructor(T)', mute='concept-name')

add('is_swappable_with', impl(
	'    template <typename T, ca::usz N>\n'
	'    using array_ref = T (&)[N];',
	'    template <typename T, typename U>\n'
	'    concept is_adl_swappable = requires(T&& t, U&& u) {\n'
	'        swap(static_cast<T&&>(t), static_cast<U&&>(u)); // qualify: ignore\n'
	'    };',
	var('swappable_with', 'ca::impl::is_adl_swappable<T, U>\n'
		'        || (ca::is_lvalue_reference<T> && ca::is_same<T, U>\n'
		'        && ca::is_move_constructible<ca::remove_reference<T>>\n'
		'        && ca::is_move_assignable<ca::remove_reference<T>>)', tp=TU),
	var('swappable_with', 'ca::impl::is_adl_swappable<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>>\n'
		'        || ca::impl::swappable_with<T&, T&>', tp='typename T, ca::usz N', spec='<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>>')),
	concept('is_swappable_with', 'ca::impl::swappable_with<T, U> && ca::impl::swappable_with<U, T>', TU))
add('is_swappable', concept('is_swappable',
	'ca::is_swappable_with<ca::add_lvalue_reference<T>, ca::add_lvalue_reference<T>>'))
add('is_nothrow_swappable_with', impl(
	'    template <typename T, typename U>\n'
	'    concept is_adl_nothrow_swappable = requires(T&& t, U&& u) {\n'
	'        { swap(static_cast<T&&>(t), static_cast<U&&>(u)) } noexcept; // qualify: ignore\n'
	'    };',
	var('nothrow_swappable_with', '(ca::impl::is_adl_swappable<T, U> && ca::impl::is_adl_nothrow_swappable<T, U>)\n'
		'        || (!ca::impl::is_adl_swappable<T, U> && ca::is_lvalue_reference<T> && ca::is_same<T, U>\n'
		'        && ca::is_nothrow_move_constructible<ca::remove_reference<T>>\n'
		'        && ca::is_nothrow_move_assignable<ca::remove_reference<T>>)', tp=TU),
	var('nothrow_swappable_with', '(ca::impl::is_adl_swappable<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>>\n'
		'        && ca::impl::is_adl_nothrow_swappable<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>>)\n'
		'        || (!ca::impl::is_adl_swappable<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>>\n'
		'        && ca::impl::nothrow_swappable_with<T&, T&>)', tp='typename T, ca::usz N',
		spec='<ca::impl::array_ref<T, N>, ca::impl::array_ref<T, N>>')),
	concept('is_nothrow_swappable_with', 'ca::is_swappable_with<T, U>\n'
		'    && ca::impl::nothrow_swappable_with<T, U> && ca::impl::nothrow_swappable_with<U, T>', TU))
add('is_nothrow_swappable', concept('is_nothrow_swappable',
	'ca::is_nothrow_swappable_with<ca::add_lvalue_reference<T>, ca::add_lvalue_reference<T>>'))


# -- property queries --------------------------------------------------------

add('alignment_of', var('alignment_of', 'alignof(T)', 'ca::usz', level=0))
add('rank', impl(
	var('rank', '0U', 'ca::usz'),
	var('rank', 'ca::impl::rank<T> + 1U', 'ca::usz', spec='<T[]>'),
	var('rank', 'ca::impl::rank<T> + 1U', 'ca::usz', 'typename T, ca::usz N', '<T[N]>')),
	var('rank', 'ca::impl::rank<T>', 'ca::usz', level=0))
add('extent', impl(
	var('extent', '0U', 'ca::usz', 'typename T, unsigned I'),
	var('extent', 'ca::impl::extent<T, I - 1U>', 'ca::usz', 'typename T, unsigned I', '<T[], I>'),
	var('extent', 'ca::impl::extent<T, I - 1U>', 'ca::usz', 'typename T, ca::usz N, unsigned I', '<T[N], I>'),
	var('extent', '0U', 'ca::usz', 'typename T', '<T[], 0U>'),
	var('extent', 'N', 'ca::usz', 'typename T, ca::usz N', '<T[N], 0U>')),
	var('extent', 'ca::impl::extent<T, I>', 'ca::usz', 'typename T, unsigned I = 0U', level=0))


# -- type relations ----------------------------------------------------------

add('is_same', concept('is_same', '__is_same(T, U)', TU))
builtin('is_base_of', '__is_base_of(B, D)', 'typename B, typename D')
add('is_virtual_base_of', '\n'.join([
	'#if ca_has_builtin(__builtin_is_virtual_base_of)',
	concept('is_virtual_base_of', '__builtin_is_virtual_base_of(B, D)', 'typename B, typename D'),
	'#else',
	impl('    template <typename B, typename D>\n'
		'    concept is_virtual_base = ca::is_convertible<D*, B*> && !requires(B* b) {\n'
		'        static_cast<D*>(b);\n'
		'    };'),
	'',
	'',
	concept('is_virtual_base_of', 'ca::is_base_of<B, D> && !ca::is_same<ca::remove_cv<B>, ca::remove_cv<D>>\n'
		'    && ca::impl::is_virtual_base<ca::remove_cv<B>, ca::remove_cv<D>>', 'typename B, typename D'),
	'#endif',
]))
FT = 'typename From, typename To'
builtin('is_convertible', '__is_convertible(From, To)', FT)
add('is_nothrow_convertible', impl(
	'    template <typename T>\n'
	'    auto convert_to(T) noexcept -> void;'),
	concept('is_nothrow_convertible', 'ca::is_convertible<From, To>\n'
		'    && (ca::is_void<To> || noexcept(ca::impl::convert_to<To>(ca::declval<From>())))', FT))
add('is_layout_compatible', '\n'.join([
	'#if ca_has_builtin(__is_layout_compatible)',
	concept('is_layout_compatible', '__is_layout_compatible(T, U)', TU),
	'#else',
	impl('    template <typename T, typename U>\n'
		'    concept are_distinct_classes = (ca::is_class<T> || ca::is_union<T>) && (ca::is_class<U> || ca::is_union<U>)\n'
		'        && !ca::is_same<ca::remove_cv<T>, ca::remove_cv<U>>;',
		struct('layout_unknown', TU, members=[
			'static_assert(false, "ca: is_layout_compatible on distinct class types needs __is_layout_compatible");',
			'static constexpr bool value = false;'])),
	'',
	'',
	concept('is_layout_compatible', 'ca::is_same<ca::remove_cv<T>, ca::remove_cv<U>>\n'
		'    || (ca::is_enum<T> && ca::is_enum<U> && ca::is_same<ca::underlying_type<T>, ca::underlying_type<U>>)\n'
		'    || (ca::impl::are_distinct_classes<T, U> && ca::impl::layout_unknown<T, U>::value)', TU),
	'#endif',
]))
add('is_pointer_interconvertible_base_of', concept('is_pointer_interconvertible_base_of',
	'(ca::is_class<B> && ca::is_same<ca::remove_cv<B>, ca::remove_cv<D>>)\n'
	'    || (ca::is_base_of<B, D> && ca::is_standard_layout<D>)', 'typename B, typename D'))

FA = 'typename F, typename... Args'
add('is_invocable', concept('is_invocable', 'requires {\n    typename ca::invoke_result<F, Args...>;\n}', FA))
add('is_invocable_r', impl(
	'    template <typename R, typename T>\n'
	'    concept is_invoke_convertible = ca::is_void<R> || ca::is_same<T, R>\n'
	'        || (ca::is_convertible<T, R> && !ca::reference_converts_from_temporary<R, T>);'),
	concept('is_invocable_r', 'ca::is_invocable<F, Args...>\n'
		'    && ca::impl::is_invoke_convertible<R, ca::invoke_result<F, Args...>>', 'typename R, ' + FA))
add('is_nothrow_invocable', concept('is_nothrow_invocable', 'ca::is_invocable<F, Args...>\n'
	'    && noexcept(ca::impl::invoke(ca::declval<F>(), ca::declval<Args>()...))', FA))
add('is_nothrow_invocable_r', impl(
	'    template <typename R, typename T>\n'
	'    concept is_invoke_nothrow_convertible = ca::is_void<R> || ca::is_same<T, R>\n'
	'        || (ca::is_nothrow_convertible<T, R> && !ca::reference_converts_from_temporary<R, T>);'),
	concept('is_nothrow_invocable_r', 'ca::is_nothrow_invocable<F, Args...>\n'
		'    && ca::impl::is_invoke_nothrow_convertible<R, ca::invoke_result<F, Args...>>', 'typename R, ' + FA))
builtin('reference_constructs_from_temporary', '__reference_constructs_from_temporary(T, U)', TU, 'concept-name')
add('reference_converts_from_temporary', '\n'.join([
	'#if ca_has_builtin(__reference_converts_from_temporary)',
	concept('reference_converts_from_temporary', '__reference_converts_from_temporary(T, U)', TU, 'concept-name'),
	'#else',
	concept('reference_converts_from_temporary',
		'ca::reference_constructs_from_temporary<T, U> && ca::is_convertible<U, T>', TU, 'concept-name'),
	'#endif',
]))


# -- type transformations ----------------------------------------------------

add('remove_cv', alias('remove_cv', '__remove_cv(T)'))
type_struct('remove_const', 'T', [('typename T', 'const T', 'T')])
type_struct('remove_volatile', 'T', [('typename T', 'volatile T', 'T')])
add('add_cv', alias('add_cv', 'const volatile T'))
add('add_const', alias('add_const', 'const T'))
add('add_volatile', alias('add_volatile', 'volatile T'))
type_struct('remove_reference', 'T', [('typename T', 'T&', 'T'), ('typename T', 'T&&', 'T')])
type_struct('add_lvalue_reference', 'T', [('typename T', 'T', 'T&', 'ca::impl::is_referenceable<T>')])
type_struct('add_rvalue_reference', 'T', [('typename T', 'T', 'T&&', 'ca::impl::is_referenceable<T>')])

for kind, std in (('signed', ['signed char', 'short', 'int', 'long', 'long long']),
		('unsigned', ['unsigned char', 'unsigned short', 'unsigned int', 'unsigned long', 'unsigned long long'])):
	sized = 'ca::conditional<sizeof(T) == sizeof(' + std[0] + '), ' + std[0] + ',\n'
	for t in std[1:-1]:
		sized += f'        ca::conditional<sizeof(T) == sizeof({t}), {t},\n'
	sized += '        ' + std[-1] + '>' * (len(std) - 1)
	groups = [('char, signed char, unsigned char', std[0]), ('short, unsigned short', std[1]),
		('int, unsigned int', std[2]), ('long, unsigned long', std[3]), ('long long, unsigned long long', std[4])]
	ranked = ''
	for i, (g, t) in enumerate(groups):
		ranked += ('' if i == 0 else '        ') + f'ca::conditional<ca::impl::is_any_of<T, {g}>, {t},\n'
	ranked += f'        ca::impl::make_{kind}_sized<T>' + '>' * len(groups)
	add(f'make_{kind}', impl(
		f'    template <typename T>\n    using make_{kind}_sized = {sized};',
		f'    template <typename T>\n    using make_{kind} = {ranked};'),
		alias(f'make_{kind}', f'ca::impl::copy_cv<T, ca::impl::make_{kind}<ca::remove_cv<T>>>',
			req='(ca::is_integral<T> && !ca::is_same<ca::remove_cv<T>, bool>) || ca::is_enum<T>'))

type_struct('remove_extent', 'T', [('typename T', 'T[]', 'T'), ('typename T, ca::usz N', 'T[N]', 'T')])
type_struct('remove_all_extents', 'T', [('typename T', 'T[]', 'typename ca::impl::remove_all_extents<T>::type'),
	('typename T, ca::usz N', 'T[N]', 'typename ca::impl::remove_all_extents<T>::type')])
type_struct('remove_pointer', 'T', [('typename T', p, 'T') for p in ('T*', 'T* const', 'T* volatile', 'T* const volatile')])
add('add_pointer', impl(
	'    template <typename T>\n'
	'    concept is_pointable = requires {\n'
	'        typename ca::type_identity<ca::remove_reference<T>*>;\n'
	'    };',
	struct('add_pointer', 'typename T', members=['using type = T;']),
	struct('add_pointer', 'typename T', 'T', ['using type = ca::remove_reference<T>*;'], 'ca::impl::is_pointable<T>')),
	alias('add_pointer', 'typename ca::impl::add_pointer<T>::type'))

add('aligned_storage', impl(
	'    template <ca::usz L, ca::usz A>\n'
	'    struct aligned_storage final {\n'
	'            using self = ca::impl::aligned_storage<L, A>;\n'
	'            struct type final {\n'
	'                    using self = typename ca::impl::aligned_storage<L, A>::type;\n'
	'                    alignas(A) unsigned char _data[L];\n'
	'            }; // struct type\n'
	'            aligned_storage(void) = delete;\n'
	'    }; // struct aligned_storage'),
	alias('aligned_storage', 'typename ca::impl::aligned_storage<L, A>::type', 'ca::usz L, ca::usz A = ca::max_alignment'))
add('aligned_union', impl(
	var('max_of', '0U', 'ca::usz', 'ca::usz... Ns'),
	var('max_of', '(N > ca::impl::max_of<Ns...>) ? N : ca::impl::max_of<Ns...>', 'ca::usz',
		'ca::usz N, ca::usz... Ns', '<N, Ns...>')),
	alias('aligned_union', 'ca::aligned_storage<ca::impl::max_of<L, sizeof(T), sizeof(Ts)...>,\n'
		'    ca::impl::max_of<alignof(T), alignof(Ts)...>>', 'ca::usz L, typename T, typename... Ts'))

add('decay', impl(
	struct('decay', 'typename T', members=['using type = ca::remove_cv<T>;']),
	struct('decay', 'typename T', 'T', ['using type = ca::remove_extent<T>*;'], 'ca::is_array<T>'),
	struct('decay', 'typename T', 'T', ['using type = ca::add_pointer<T>;'], 'ca::is_function<T>')),
	alias('decay', 'typename ca::impl::decay<ca::remove_reference<T>>::type'))
add('remove_cvref', alias('remove_cvref', '__remove_cvref(T)'))
add('enable_if', impl(
	struct('enable_if', 'bool B, typename T'),
	struct('enable_if', 'typename T', 'true, T', ['using type = T;'])),
	alias('enable_if', 'typename ca::impl::enable_if<B, T>::type', 'bool B, typename T = void'))
type_struct('conditional', 'T', [('typename T, typename F', 'false, T, F', 'F')], 'bool B, typename T, typename F')

DECAYED = 'ca::is_same<T, ca::decay<T>> && ca::is_same<U, ca::decay<U>>'
add('common_type', impl(
	'#pragma GCC diagnostic push\n'
	'#pragma GCC diagnostic ignored "-Wextra"\n'
	'#pragma GCC diagnostic ignored "-Wenum-compare"\n'
	'#pragma GCC diagnostic ignored "-Wdeprecated"\n'
	'#pragma GCC diagnostic ignored "-Wdeprecated-enum-float-conversion"\n'
	'#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"\n'
	'    template <typename T, typename U>\n'
	'    using ternary = decltype(false ? ca::declval<T>() : ca::declval<U>());\n#pragma GCC diagnostic pop',
	'    template <typename T, typename U>\n'
	'    concept is_ternary_valid = requires {\n'
	'        typename ca::impl::ternary<T, U>;\n'
	'    };',
	struct('common_type', 'typename... Ts'),
	struct('common_type', 'typename T', 'T', ['using type = typename ca::impl::common_type<T, T>::type;'],
		'requires {\n        typename ca::impl::common_type<T, T>::type;\n    }'),
	struct('common_type', TU, 'T, U', ['using type = typename ca::impl::common_type<ca::decay<T>, ca::decay<U>>::type;'],
		f'requires {{\n        typename ca::impl::common_type<ca::decay<T>, ca::decay<U>>::type;\n    }}\n'
		f'        && (!({DECAYED}))'),
	struct('common_type', TU, 'T, U', ['using type = ca::decay<ca::impl::ternary<T, U>>;'],
		f'ca::impl::is_ternary_valid<T, U>\n        && {DECAYED}'),
	struct('common_type', TU, 'T, U', ['using type = ca::remove_cvref<ca::impl::ternary<const T&, const U&>>;'],
		f'ca::impl::is_ternary_valid<const T&, const U&> && (!ca::impl::is_ternary_valid<T, U>)\n        && {DECAYED}'),
	struct('common_type', 'typename T, typename U, typename V, typename... Ws', 'T, U, V, Ws...',
		['using type = typename ca::impl::common_type<typename ca::impl::common_type<T, U>::type, V, Ws...>::type;'],
		'requires {\n        typename ca::impl::common_type<typename ca::impl::common_type<T, U>::type, V, Ws...>::type;\n    }')),
	alias('common_type', 'typename ca::impl::common_type<Ts...>::type', 'typename... Ts'))

add('basic_common_reference', '\n'.join([
	'template <typename T, typename U, template <typename> typename TQ, template <typename> typename UQ>',
	'struct basic_common_reference final {',
	'        using self = ca::basic_common_reference<T, U, TQ, UQ>;',
	'        basic_common_reference(void) = delete;',
	'}; // struct basic_common_reference',
]))
CR1 = 'ca::impl::common_ref<T, U>::type'
BCR = ('ca::basic_common_reference<ca::remove_cvref<T>, ca::remove_cvref<U>,\n'
	'        ca::impl::xref<T>::template type, ca::impl::xref<U>::template type>::type')
LL = 'ca::impl::cond_res<ca::impl::copy_cv<T, U>&, ca::impl::copy_cv<U, T>&>'
add('common_reference', impl(
	'    template <typename T, typename U>\n'
	'    using copy_cvref = ca::conditional<ca::is_lvalue_reference<T>,\n'
	'        ca::add_lvalue_reference<ca::impl::copy_cv<ca::remove_reference<T>, U>>,\n'
	'        ca::conditional<ca::is_rvalue_reference<T>,\n'
	'        ca::add_rvalue_reference<ca::impl::copy_cv<ca::remove_reference<T>, U>>,\n'
	'        ca::impl::copy_cv<T, U>>>;',
	'    template <typename T>\n'
	'    struct xref final {\n'
	'            using self = ca::impl::xref<T>;\n'
	'            template <typename U>\n'
	'            using type = ca::impl::copy_cvref<T, U>;\n'
	'            xref(void) = delete;\n'
	'    }; // struct xref',
	'#pragma GCC diagnostic push\n'
	'#pragma GCC diagnostic ignored "-Wextra"\n'
	'#pragma GCC diagnostic ignored "-Wenum-compare"\n'
	'#pragma GCC diagnostic ignored "-Wdeprecated"\n'
	'#pragma GCC diagnostic ignored "-Wdeprecated-enum-float-conversion"\n'
	'#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"\n'
	'    template <typename T, typename U>\n'
	'    using cond_res = decltype(false ? ca::declval<T (&)(void)>()() : ca::declval<U (&)(void)>()());\n#pragma GCC diagnostic pop',
	struct('common_ref', TU),
	struct('common_ref', TU, 'T&, U&', [f'using type = {LL};'],
		f'requires {{\n        typename {LL};\n    }}\n        && ca::is_reference<{LL}>'),
	struct('common_ref', TU, 'T&&, U&&', ['using type = ca::remove_reference<typename ca::impl::common_ref<T&, U&>::type>&&;'],
		'requires {\n        typename ca::impl::common_ref<T&, U&>::type;\n    }\n'
		'        && ca::is_convertible<T&&, ca::remove_reference<typename ca::impl::common_ref<T&, U&>::type>&&>\n'
		'        && ca::is_convertible<U&&, ca::remove_reference<typename ca::impl::common_ref<T&, U&>::type>&&>'),
	struct('common_ref', TU, 'T&&, U&', ['using type = typename ca::impl::common_ref<const T&, U&>::type;'],
		'requires {\n        typename ca::impl::common_ref<const T&, U&>::type;\n    }\n'
		'        && ca::is_convertible<T&&, typename ca::impl::common_ref<const T&, U&>::type>'),
	struct('common_ref', TU, 'T&, U&&', ['using type = typename ca::impl::common_ref<U&&, T&>::type;'],
		'requires {\n        typename ca::impl::common_ref<U&&, T&>::type;\n    }'),
	'    template <typename T, typename U>\n'
	'    concept is_common_ref_valid = ca::is_reference<T> && ca::is_reference<U>\n'
	'        && requires {\n'
	f'            typename {CR1};\n'
	'        }\n'
	f'        && ca::is_convertible<ca::add_pointer<T>, ca::add_pointer<typename {CR1}>>\n'
	f'        && ca::is_convertible<ca::add_pointer<U>, ca::add_pointer<typename {CR1}>>;',
	'    template <typename T, typename U>\n'
	'    concept is_basic_common_ref_valid = requires {\n'
	f'        typename {BCR};\n'
	'    };',
	'    template <typename T, typename U>\n'
	'    concept is_cond_res_valid = requires {\n'
	'        typename ca::impl::cond_res<T, U>;\n'
	'    };',
	'    template <typename T, typename U>\n'
	'    concept is_common_type_valid = requires {\n'
	'        typename ca::common_type<T, U>;\n'
	'    };',
	struct('common_reference', 'typename... Ts'),
	struct('common_reference', 'typename T', 'T', ['using type = T;']),
	struct('common_reference', TU, 'T, U', [f'using type = typename {CR1};'], 'ca::impl::is_common_ref_valid<T, U>'),
	struct('common_reference', TU, 'T, U', [f'using type = typename {BCR};'],
		'(!ca::impl::is_common_ref_valid<T, U>) && ca::impl::is_basic_common_ref_valid<T, U>'),
	struct('common_reference', TU, 'T, U', ['using type = ca::impl::cond_res<T, U>;'],
		'(!ca::impl::is_common_ref_valid<T, U>) && (!ca::impl::is_basic_common_ref_valid<T, U>)\n'
		'        && ca::impl::is_cond_res_valid<T, U>'),
	struct('common_reference', TU, 'T, U', ['using type = ca::common_type<T, U>;'],
		'(!ca::impl::is_common_ref_valid<T, U>) && (!ca::impl::is_basic_common_ref_valid<T, U>)\n'
		'        && (!ca::impl::is_cond_res_valid<T, U>) && ca::impl::is_common_type_valid<T, U>'),
	struct('common_reference', 'typename T, typename U, typename V, typename... Ws', 'T, U, V, Ws...',
		['using type = typename ca::impl::common_reference<typename ca::impl::common_reference<T, U>::type, V, Ws...>::type;'],
		'requires {\n        typename ca::impl::common_reference<typename ca::impl::common_reference<T, U>::type, V, Ws...>::type;\n    }')),
	alias('common_reference', 'typename ca::impl::common_reference<Ts...>::type', 'typename... Ts'))

add('underlying_type', alias('underlying_type', '__underlying_type(T)', req='ca::is_enum<T>'))

INV = '(static_cast<Args&&>(args)...)'
add('invoke_result', impl(
	'    template <typename C, typename T>\n'
	'    concept is_invoke_on_object = ca::is_base_of<C, ca::remove_cvref<T>>;',
	'    template <typename T>\n'
	'    concept is_invoke_on_wrapper = !ca::is_same<ca::unwrap_reference<ca::remove_cvref<T>>, ca::remove_cvref<T>>;',
	'    template <typename F, typename... Args>\n'
	'    auto invoke(F&& f, Args&&... args)\n'
	f'        noexcept(noexcept(static_cast<F&&>(f){INV}))\n'
	f'        -> decltype(static_cast<F&&>(f){INV});',
	*[f'    template <typename M, typename C, typename T, typename... Args>\n'
	f'    requires ca::is_function<M> && {req}\n'
	f'    auto invoke(M C::* f, T&& t, Args&&... args)\n'
	f'        noexcept(noexcept(({obj}.*f){INV}))\n'
	f'        -> decltype(({obj}.*f){INV});' for req, obj in (
		('ca::impl::is_invoke_on_object<C, T>', 'static_cast<T&&>(t)'),
		('(!ca::impl::is_invoke_on_object<C, T>) && ca::impl::is_invoke_on_wrapper<T>', 'static_cast<T&&>(t).get()'),
		('(!ca::impl::is_invoke_on_object<C, T>) && (!ca::impl::is_invoke_on_wrapper<T>)', '(*static_cast<T&&>(t))'))],
	*[f'    template <typename M, typename C, typename T>\n'
	f'    requires (!ca::is_function<M>) && {req}\n'
	f'    auto invoke(M C::* f, T&& t)\n'
	f'        noexcept(noexcept({obj}.*f))\n'
	f'        -> decltype({obj}.*f);' for req, obj in (
		('ca::impl::is_invoke_on_object<C, T>', 'static_cast<T&&>(t)'),
		('(!ca::impl::is_invoke_on_object<C, T>) && ca::impl::is_invoke_on_wrapper<T>', 'static_cast<T&&>(t).get()'),
		('(!ca::impl::is_invoke_on_object<C, T>) && (!ca::impl::is_invoke_on_wrapper<T>)', '(*static_cast<T&&>(t))'))]),
	alias('invoke_result', 'decltype(ca::impl::invoke(ca::declval<F>(), ca::declval<Args>()...))', FA))
add('void_t', alias('void_t', 'void', 'typename...'))
type_struct('type_identity', 'T', [])
type_struct('unwrap_reference', 'T', [])
add('unwrap_ref_decay', alias('unwrap_ref_decay', 'ca::unwrap_reference<ca::decay<T>>'))


# -- logical operations ------------------------------------------------------

for name, unit, short, step in (('conjunction', 'true', 'false', 'true'), ('disjunction', 'false', 'true', 'false')):
	add(name, impl(
		var(name, unit, tp='typename... Bs'),
		var(f'{name}_from', short, tp='bool V, typename... Bs'),
		var(f'{name}_from', f'ca::impl::{name}<Bs...>', tp='typename... Bs', spec=f'<{step}, Bs...>'),
		var(name, f'ca::impl::{name}_from<static_cast<bool>(B::value), Bs...>', tp='typename B, typename... Bs',
			spec='<B, Bs...>')),
		concept(name, f'ca::impl::{name}<Bs...>', 'typename... Bs', 'concept-name'))
add('negation', concept('negation', '!static_cast<bool>(B::value)', 'typename B', 'concept-name'))


# -- member relationships ----------------------------------------------------

def guarded_fn(name, builtin_name, tp, params, unnamed, call, spec='constexpr'):
	return '\n'.join([
		f'#if ca_has_builtin({builtin_name})',
		f'template <{tp}>',
		f'{spec} auto {name}({params}) noexcept -> bool {{',
		f'    return {builtin_name}({call});',
		'}',
		'#else',
		f'template <{tp}>',
		f'{spec} auto {name}({unnamed}) noexcept -> bool {{',
		'',
		f'    static_assert(false, "ca: {name} needs {builtin_name}");',
		'    return false;',
		'}',
		'#endif',
	])


add('is_pointer_interconvertible_with_class', guarded_fn('is_pointer_interconvertible_with_class',
	'__builtin_is_pointer_interconvertible_with_class', 'typename S, typename M', 'M S::* member', 'M S::*', 'member'))
add('is_corresponding_member', guarded_fn('is_corresponding_member', '__builtin_is_corresponding_member',
	'typename S1, typename S2, typename M1, typename M2', 'M1 S1::* first, M2 S2::* second', 'M1 S1::*, M2 S2::*',
	'first, second'))


# -- constant evaluation context ---------------------------------------------

add('is_constant_evaluated', '\n'.join([
	'constexpr auto is_constant_evaluated(void) noexcept -> bool {',
	'    return __builtin_is_constant_evaluated();',
	'}',
]))
add('is_within_lifetime', guarded_fn('is_within_lifetime', '__builtin_is_within_lifetime', 'typename T',
	'const T* pointer', 'const T*', 'pointer', 'consteval'))


# -- impl helpers ------------------------------------------------------------

IMPL = {
	'impl/is_any_of': impl(concept('is_any_of', '(ca::is_same<T, Ts> || ...)', 'typename T, typename... Ts')
		.replace('\n', '\n    ').join(['    ', ''])),
	'impl/is_referenceable': impl(concept('is_referenceable', 'requires {\n    typename ca::type_identity<T&>;\n}')
		.replace('\n', '\n    ').join(['    ', ''])),
	'impl/copy_cv': impl(alias('copy_cv', 'ca::conditional<ca::is_const<T>,\n'
		'    ca::conditional<ca::is_volatile<T>, const volatile U, const U>,\n'
		'    ca::conditional<ca::is_volatile<T>, volatile U, U>>', TU).replace('\n', '\n    ').join(['    ', ''])),
}

EXTERNAL = {'usz': 'integers/usz', 'declval': 'utility/declval', 'max_alignment': 'memory/max_alignment'}


# -- emit --------------------------------------------------------------------

def tabify(body):
	out = []
	for line in body.split('\n'):
		if not line.strip():
			out.append('')
		elif line.startswith('#'):
			out.append(line)
		elif line.startswith('@@'):
			out.append(banner(line[2:], 3))
		else:
			n = len(line) - len(line.lstrip(' '))
			out.append('\t' * (n // 4 + 1) + ' ' * (n % 4) + line.lstrip(' '))
	return '\n'.join(out)


def title_of(name, indent):
	return ' '.join(w.upper() for w in name.split('_'))


ALL = {}
for n, b in SPECS.items():
	ALL[('traits/' + n)] = b
for n, b in IMPL.items():
	ALL['traits/' + n] = b

OWNER = {}
for path, body in ALL.items():
	for line in body.split('\n'):
		m = re.match(r'    (?:concept|using|struct|auto|inline constexpr [\w:()]+)\s+(\w+)', line)
		if m and m.group(1) not in OWNER:
			OWNER[m.group(1)] = path
PUBLIC = {p.split('/')[-1]: p for p in ALL if not p.startswith('traits/impl/')} | EXTERNAL


def includes(path, body):
	inc = set()
	for m in re.finditer(r'\bca::(?!impl::)(\w+)', body):
		if m.group(1) in PUBLIC:
			inc.add(PUBLIC[m.group(1)])
	for m in re.finditer(r'\bca::impl::(\w+)', body):
		w = m.group(1)
		if w in OWNER:
			inc.add(OWNER[w])
		elif w in PUBLIC:
			inc.add(PUBLIC[w])
	if 'ca_has_builtin' in body:
		inc.add('config/has_builtin')
	inc.discard(path)
	return sorted(inc, key=lambda p: (not p.startswith('config/'), p))


NOTES = {
	'integral_constant': 'Not final: traits derive from it.',
	'is_implicit_lifetime': 'Without __builtin_is_implicit_lifetime: approximation; aggregates whose destructor is '
		'non-trivial but not user-provided report false.',
	'is_virtual_base_of': 'Without __builtin_is_virtual_base_of: private or protected virtual bases report false.',
	'is_layout_compatible': 'Without __is_layout_compatible (clang 18): distinct class types fail to compile '
		'with a ca: message.',
	'reference_converts_from_temporary': 'Without __reference_converts_from_temporary (clang 18): '
		'reference_constructs_from_temporary && is_convertible.',
	'is_invocable_r': 'C++23 rule: a reference result bound to a temporary is not invocable.',
	'is_nothrow_invocable_r': 'C++23 rule: a reference result bound to a temporary is not invocable.',
	'is_pointer_interconvertible_with_class': 'Needs __builtin_is_pointer_interconvertible_with_class; '
		'fails to compile with a ca: message without it (clang 18).',
	'is_corresponding_member': 'Needs __builtin_is_corresponding_member; fails to compile with a ca: message '
		'without it (clang 18).',
	'is_within_lifetime': 'Needs __builtin_is_within_lifetime; fails to compile with a ca: message without it.',
	'invoke_result': 'INVOKE rules. Reference wrappers are detected through ca::impl::unwrap_reference and '
		'unwrapped with .get().',
	'unwrap_reference': 'Identity until a reference wrapper specializes ca::impl::unwrap_reference.',
	'common_type': 'Customize by specializing ca::impl::common_type<A, B> on decayed types.',
	'basic_common_reference': 'Customization point: specialize it with a member type.',
	'conjunction': 'Over types with ::value, short-circuiting.',
	'disjunction': 'Over types with ::value, short-circuiting.',
	'negation': 'Over a type with ::value.',
	'aligned_storage': 'Deprecated in std, kept for parity. Default alignment: ca::max_alignment.',
	'aligned_union': 'Deprecated in std, kept for parity. No alignment_value member.',
	'is_pod': 'Deprecated in std, kept for parity.',
	'is_trivial': 'Deprecated in std, kept for parity.',
}


def std_name(name, decl):
	if re.search(r'^\s*template <[^\n]*\n\s*(?:requires [^\n]*\n\s*)?using ', decl) or decl.lstrip().startswith('using '):
		return f'std::{name}_t' if name not in ('true_type', 'false_type', 'bool_constant', 'void_t') else f'std::{name}'
	if re.search(r'\nconcept ', decl):
		return f'std::{name}_v, as a concept'
	if 'inline constexpr' in decl:
		return f'std::{name}_v'
	return f'std::{name}'


def with_doc(path, body):
	name = path.split('/')[-1]
	guarded = body.startswith('#if')
	decl = body if guarded else body.rsplit('} // namespace impl', 1)[-1]
	text = f'{std_name(name, decl)}.'
	if name in NOTES:
		text += ' ' + NOTES[name]
	words, lines, cur = text.split(), [], '/* @DOC'
	for w in words:
		if len(cur) + 1 + len(w) > 100:
			lines.append(cur)
			cur = '   ' + w
		else:
			cur += ' ' + w
	doc = '\n'.join(lines + [cur + ' */'])
	end = '} // namespace impl'
	if guarded or end not in body:
		return doc + '\n' + body
	head, tail = body.rsplit(end, 1)
	return head + end + '\n\n\n' + doc + '\n' + tail.lstrip('\n')


for path in list(ALL):
	if not path.startswith('traits/impl/'):
		ALL[path] = with_doc(path, ALL[path])

if OPTS.clean or OPTS.fclean:
	traits = os.path.join(OUT, 'traits')
	outputs = [os.path.join(OUT, path + '.hpp') for path in ALL]
	sys.exit(cleanup.run(OPTS, 'gen_traits', cleanup.bytecode(), outputs, [os.path.join(traits, 'impl'), traits]))

count = 0
for path, body in ALL.items():
	name = path.split('/')[-1]
	guard = 'ca_primitives_' + path.replace('/', '_') + '_hpp'
	inc = includes(path, body)
	lines = [f'#ifndef {guard}', f'#define {guard}', '']
	if inc or EXTRA_SYS.get(name):
		lines += [f'#include "1-primitives/{p}.hpp"' for p in inc]
		lines += [f'#include <{s}>' for s in EXTRA_SYS.get(name, [])]
		lines += ['']
	lines += [
		'',
		banner('CA NAMESPACE', 0),
		'',
		'namespace ca {',
		'',
		'',
		banner(title_of(name, 1), 1),
		'',
		tabify(body),
		'',
		'} // namespace ca',
		'',
		f'#endif // {guard}',
		'',
	]
	dest = os.path.join(OUT, path + '.hpp')
	os.makedirs(os.path.dirname(dest), exist_ok=True)
	with open(dest, 'w') as fh:
		fh.write('\n'.join(lines))
	count += 1
print(f'{count} headers')
