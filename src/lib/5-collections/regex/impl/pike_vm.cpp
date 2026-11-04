#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/fill_bytes.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/impl/decode_at.hpp"
#include "2-vocabulary/unicode/impl/decode_before.hpp"
#include "2-vocabulary/unicode/properties_of.hpp"
#include "5-collections/regex/impl/pike_vm.hpp"
#include "5-collections/regex/impl/regex_limits.hpp"
#include "5-collections/regex/impl/regex_look.hpp"


static auto is_word_point(const char32_t point, const bool ascii) noexcept -> bool {

	if (point < 0x80U)
		return (point >= U'0' && point <= U'9') || (point >= U'A' && point <= U'Z') || (point >= U'a' && point <= U'z') || point == U'_';
	if (ascii)
		return false;
	return ca::unicode::properties_of(point).is_alnum();
}

static auto word_before(const char* const text, const ca::usz at, const bool ascii) noexcept -> bool {

	if (at == 0U)
		return false;
	const ca::unicode::decoded point = ca::unicode::impl::decode_before(text, text + at);
	return point.valid() && is_word_point(point.code_point(), ascii);
}

static auto word_after(const char* const text, const ca::usz size, const ca::usz at, const bool ascii) noexcept -> bool {

	if (at == size)
		return false;
	const ca::unicode::decoded point = ca::unicode::impl::decode_at(text + at, text + size);
	return point.valid() && is_word_point(point.code_point(), ascii);
}

static auto holds(const ca::u8 kind, const char* const text, const ca::usz size, const ca::usz at) noexcept -> bool {

	switch (static_cast<ca::impl::regex_look>(kind)) {
		case ca::impl::regex_look::begin_text:
			return at == 0U;
		case ca::impl::regex_look::end_text:
			return at == size;
		case ca::impl::regex_look::end_text_or_newline:
			return at == size || (at + 1U == size && text[at] == '\n');
		case ca::impl::regex_look::begin_line:
			return at == 0U || text[at - 1U] == '\n';
		case ca::impl::regex_look::end_line:
			return at == size || text[at] == '\n';
		case ca::impl::regex_look::word:
			return word_before(text, at, false) != word_after(text, size, at, false);
		case ca::impl::regex_look::not_word:
			return word_before(text, at, false) == word_after(text, size, at, false);
		case ca::impl::regex_look::word_ascii:
			return word_before(text, at, true) != word_after(text, size, at, true);
		case ca::impl::regex_look::not_word_ascii:
			return word_before(text, at, true) == word_after(text, size, at, true);
	}
	return false;
}

[[gnu::always_inline]] static inline auto has(const ca::impl::regex_threads& list, const ca::u32 key) noexcept -> bool {

	const ca::u32 index = list._sparse[key];
	return index < list._count && list._dense[index] == key;
}

[[gnu::always_inline]] static inline auto insert(ca::impl::regex_threads& list, const ca::u32 key) noexcept -> void {

	       list._sparse[key] = list._count;
	list._dense[list._count] = key;
	++list._count;
}

[[gnu::always_inline]] static inline auto next_of(const ca::impl::regex_inst& inst, const ca::impl::regex_transition* const transitions, const ca::u8 unit) noexcept -> ca::u32 {

	const ca::impl::regex_transition* const at = transitions + inst._x;
	const ca::u32 count = inst._y;
	if (count <= 8U) {
		for (ca::u32 i = 0U; i < count; ++i) {
			if (unit < at[i]._low)
				return ca::impl::regex_none;
			if (unit <= at[i]._high)
				return at[i]._next;
		}
		return ca::impl::regex_none;
	}
	ca::u32 low = 0U;
	ca::u32 high = count;
	while (low < high) {
		const ca::u32 middle = low + (high - low) / 2U;
		if (at[middle]._high < unit)
			low = middle + 1U;
		else
			high = middle;
	}
	if (low < count && at[low]._low <= unit)
		return at[low]._next;
	return ca::impl::regex_none;
}

[[gnu::always_inline]] static inline auto key_of(const ca::impl::regex_program& program, const ca::usz* const current, const ca::u32 pc, const ca::usz at) noexcept -> ca::u32 {

	if (program._shift == 0U)
		return pc;
	const ca::u32 guard = program._guards[pc];
	return pc << 1U | (guard != ca::impl::regex_none && current[guard] == at ? 1U : 0U);
}

static auto add(const ca::impl::regex_program& program, const ca::impl::regex_scratch& scratch, ca::impl::regex_threads& list, const ca::u32 start, const ca::impl::regex_search& search, const ca::usz at) noexcept -> void {

	const ca::impl::regex_inst* const insts = program._insts;
	const ca::u32 slots = program._slots;
	ca::usz* const current = scratch._current;
	ca::impl::regex_frame* const stack = scratch._stack;
	ca::usz top = 1U;
	stack[0] = ca::impl::regex_frame{0U, start, ca::impl::regex_none};
	while (top != 0U) {
		const ca::impl::regex_frame frame = stack[--top];
		if (frame._slot != ca::impl::regex_none) {
			current[frame._slot] = frame._value;
			continue;
		}
		ca::u32 pc = frame._target;
		while (true) {
			if (insts[pc]._op == ca::impl::regex_op::progress) {
				pc = current[insts[pc]._y] == at ? insts[pc]._x : insts[pc]._z;
				continue;
			}
			const ca::u32 key = key_of(program, current, pc, at);
			if (has(list, key))
				break;
			insert(list, key);
			const ca::impl::regex_inst& inst = insts[pc];
			if (inst._op == ca::impl::regex_op::split) {
				stack[top++] = ca::impl::regex_frame{0U, inst._y, ca::impl::regex_none};
				          pc = inst._x;
			}
			else if (inst._op == ca::impl::regex_op::save) {
				    stack[top++] = ca::impl::regex_frame{current[inst._y], 0U, inst._y};
				current[inst._y] = at;
				              pc = inst._x;
			}
			else if (inst._op == ca::impl::regex_op::look) {
				if (!holds(inst._look, search._text, search._size, at))
					break;
				pc = inst._x;
			}
			else {
				ca::copy_bytes(list._slots + static_cast<ca::usz>(key) * slots, current, slots * sizeof(ca::usz));
				break;
			}
		}
	}
}

static auto skip(const ca::impl::regex_program& program, const char* const text, const ca::usz size, ca::usz at) noexcept -> ca::usz {

	if (program._prefix_size != 0U) {
		const ca::optional<ca::usz> hit = ca::sv::str{text + at, size - at}.find(ca::sv::str{program._prefix, program._prefix_size});
		return hit ? at + *hit : ca::impl::regex_no_position;
	}
	if (!program._skips)
		return at;
	const ca::u64* const firsts = program._first_bytes;
	while (at < size) {
		const ca::u8 unit = static_cast<ca::u8>(text[at]);
		if ((firsts[unit >> 6U] >> (unit & 63U) & 1U) != 0U)
			return at;
		++at;
	}
	return ca::impl::regex_no_position;
}

[[gnu::always_inline]] static inline auto is_boundary(const char* const text, const ca::usz size, const ca::usz at) noexcept -> bool {
	return at == size || (static_cast<ca::u8>(text[at]) & 0xC0U) != 0x80U;
}

auto ca::impl::regex_scratch_at(void* const block, const ca::usz keys, const ca::u32 slots) noexcept -> ca::impl::regex_scratch {

	const ca::usz thread_slots = keys * slots;
	ca::usz* const words = static_cast<ca::usz*>(block);
	ca::impl::regex_scratch scratch{};
	 scratch._lists[0]._slots = words;
	 scratch._lists[1]._slots = words + thread_slots;
	         scratch._current = words + 2U * thread_slots;
	            scratch._best = scratch._current + slots;
	           scratch._stack = reinterpret_cast<ca::impl::regex_frame*>(scratch._best + slots);
	ca::u32* const indices = reinterpret_cast<ca::u32*>(scratch._stack + keys + 1U);
	 scratch._lists[0]._dense = indices;
	scratch._lists[0]._sparse = indices + keys;
	 scratch._lists[1]._dense = indices + 2U * keys;
	scratch._lists[1]._sparse = indices + 3U * keys;
	ca::fill_bytes(indices, 0U, 4U * keys * sizeof(ca::u32));
	return scratch;
}

auto ca::impl::regex_execute(const ca::impl::regex_program& program, const ca::impl::regex_scratch& scratch, const ca::impl::regex_search& search) noexcept -> bool {

	const char* const text = search._text;
	const ca::usz size = search._size;
	const ca::u32 slots = program._slots;
	const bool anchored = search._anchored || program._anchored;
	ca::impl::regex_threads lists[2] = {scratch._lists[0], scratch._lists[1]};
	ca::impl::regex_threads* now = &lists[0];
	ca::impl::regex_threads* later = &lists[1];
	  now->_count = 0U;
	later->_count = 0U;
	bool matched = false;
	ca::usz at = search._from;
	while (true) {
		if (!matched && (!anchored || at == search._from)) {
			if (now->_count == 0U && !anchored) {
				at = skip(program, text, size, at);
				if (at == ca::impl::regex_no_position)
					break;
			}
			if (is_boundary(text, size, at)) {
				ca::fill_bytes(scratch._current, 0xFFU, slots * sizeof(ca::usz));
				scratch._current[0] = at;
				add(program, scratch, *now, program._start, search, at);
			}
		}
		if (now->_count == 0U) {
			if (matched || anchored || at >= size)
				break;
			++at;
			continue;
		}
		const bool more = at < size;
		const ca::u8 unit = more ? static_cast<ca::u8>(text[at]) : 0U;
		for (ca::u32 i = 0U; i < now->_count; ++i) {
			const ca::u32 key = now->_dense[i];
			const ca::impl::regex_inst& inst = program._insts[key >> program._shift];
			const ca::usz* const thread = now->_slots + static_cast<ca::usz>(key) * slots;
			if (inst._op == ca::impl::regex_op::match) {
				if (search._full && at != size)
					continue;
				if (at == search._forbid && thread[0] == at)
					continue;
				ca::copy_bytes(scratch._best, thread, slots * sizeof(ca::usz));
				scratch._best[1] = at;
				         matched = true;
				break;
			}
			if (!more || inst._op != ca::impl::regex_op::bytes)
				continue;
			const ca::u32 target = next_of(inst, program._transitions, unit);
			if (target == ca::impl::regex_none)
				continue;
			ca::copy_bytes(scratch._current, thread, slots * sizeof(ca::usz));
			add(program, scratch, *later, target, search, at + 1U);
		}
		ca::impl::regex_threads* const done = now;
		          now = later;
		        later = done;
		later->_count = 0U;
		if (!more)
			break;
		++at;
	}
	return matched;
}
