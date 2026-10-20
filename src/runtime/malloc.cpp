/* malloc, calloc, realloc, free and the aligned variants for programs linked without libc (see
   start_x86_64.cpp), on ca::heap. ca::heap's deallocate takes the block's size, which free does
   not get: each block starts with a 16-byte header holding the allocation's size and the
   offset back to its start, the pointer handed out right after it (16-aligned, as glibc's). */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

static constexpr ca::usz HEADER = 16U;

/* The header right before user: {allocation size, distance from the allocation's start}. */
static auto header_of(void* const user) noexcept -> ca::u64* {
	return reinterpret_cast<ca::u64*>(static_cast<char*>(user) - HEADER);
}

static auto out_of_memory(void) noexcept -> void* {

	ca::sys::set_last_error(ca::errc::out_of_memory);
	return nullptr;
}

/* size bytes aligned to alignment (a power of two, 16 at least), zeroed or not. */
static auto take(const ca::usz size, const ca::usz alignment, const bool zeroed) noexcept -> void* {

	const ca::usz slack = alignment > HEADER ? alignment : 0U;
	const ca::usz total = size + HEADER + slack;
	if (total < size) [[unlikely]]
		return out_of_memory();
	const ca::heap heap;
	auto block = zeroed ? heap.allocate_zeroed(total, ca::heap::ALIGNMENT) : heap.allocate(total, ca::heap::ALIGNMENT);
	if (!block) [[unlikely]]
		return out_of_memory();
	char* const start = static_cast<char*>(block->data());
	ca::usz offset = HEADER;
	if (slack != 0U)
		offset = ((reinterpret_cast<ca::usz>(start) + HEADER + alignment - 1U) & ~(alignment - 1U)) - reinterpret_cast<ca::usz>(start);
	void* const user = start + offset;
	header_of(user)[0] = block->size();
	header_of(user)[1] = offset;
	return user;
}

static auto give_back(void* const user) noexcept -> void {

	const ca::u64* const header = header_of(user);
	char* const start = static_cast<char*>(user) - header[1];
	ca::heap{}.deallocate(ca::allocation{start, static_cast<ca::usz>(header[0])}, ca::heap::ALIGNMENT);
}

extern "C" {

	auto malloc(const ca::usz size) noexcept -> void* { // ns-leak: ignore
		return take(size, HEADER, false);
	}

	auto calloc(const ca::usz count, const ca::usz size) noexcept -> void* { // ns-leak: ignore

		ca::usz total;
		if (__builtin_mul_overflow(count, size, &total)) [[unlikely]]
			return out_of_memory();
		return take(total, HEADER, true);
	}

	auto free(void* const user) noexcept -> void { // ns-leak: ignore
		if (user != nullptr)
			give_back(user);
	}

	auto realloc(void* const user, const ca::usz size) noexcept -> void* { // ns-leak: ignore

		if (user == nullptr)
			return take(size, HEADER, false);
		const ca::u64* const header = header_of(user);
		const ca::usz usable = static_cast<ca::usz>(header[0] - header[1]);
		if (size <= usable && size >= usable / 2U)
			return user;
		void* const moved = take(size, HEADER, false);
		if (moved == nullptr) [[unlikely]]
			return nullptr;
		ca::copy_bytes(moved, user, size < usable ? size : usable);
		give_back(user);
		return moved;
	}

	auto aligned_alloc(const ca::usz alignment, const ca::usz size) noexcept -> void* { // ns-leak: ignore

		if (alignment == 0U || (alignment & (alignment - 1U)) != 0U) [[unlikely]] {
			ca::sys::set_last_error(ca::errc::invalid_argument);
			return nullptr;
		}
		return take(size, alignment > HEADER ? alignment : HEADER, false);
	}

	auto posix_memalign(void** const out, const ca::usz alignment, const ca::usz size) noexcept -> int { // ns-leak: ignore

		if (alignment < sizeof(void*) || (alignment & (alignment - 1U)) != 0U) [[unlikely]]
			return ca::errc::invalid_argument.code();
		void* const block = take(size, alignment > HEADER ? alignment : HEADER, false);
		if (block == nullptr) [[unlikely]]
			return ca::errc::out_of_memory.code();
		*out = block;
		return 0;
	}

} // extern "C"

#endif
