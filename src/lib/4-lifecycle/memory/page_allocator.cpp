#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/align_up.hpp"
#include "1-primitives/numeric/min.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/memory/advice.hpp"
#include "3-system/memory/madvise.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/mremap.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/page_size.hpp"
#include "3-system/memory/protection.hpp"
#include "4-lifecycle/memory/page_allocator.hpp"


static constinit ca::atomic<ca::usz> cached_page_size{0U}; // file-state: ignore

[[nodiscard]] static auto page_size(void) noexcept -> ca::usz {

	ca::usz size = cached_page_size.load<ca::memory_order::relaxed>();
	if (size == 0U) [[unlikely]] {
		size = ca::sys::page_size();
		cached_page_size.store<ca::memory_order::relaxed>(size);
	}
	return size;
}

[[noreturn, gnu::noinline, gnu::cold]] static auto pages_neither_unmapped_nor_dropped(void) noexcept -> void {
	ca::trap();
}

[[nodiscard]] static auto pages(void* const data, const ca::usz size) noexcept -> ca::span<ca::byte> {
	return ca::span<ca::byte>{static_cast<ca::byte*>(data), size};
}

static auto release(void* const data, const ca::usz size) noexcept -> void {

	if (ca::sys::munmap(pages(data, size))) [[likely]]
		return;
	if (!ca::sys::madvise(pages(data, size), ca::sys::advice::dont_need)) [[unlikely]]
		pages_neither_unmapped_nor_dropped();
}

[[nodiscard]] static auto map_pages(const ca::usz size, const ca::usz alignment, const ca::usz page) noexcept -> ca::result<void*> {

	constexpr int read_write = ca::sys::protection::read | ca::sys::protection::write;
	ca::usz span{size};
	if (alignment > page && ca::add_overflow(span, size, alignment - page)) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	auto mapped = ca::sys::mmap_anonymous(span, read_write);
	if (!mapped) [[unlikely]]
		return mapped.propagate();
	if (alignment <= page)
		return static_cast<void*>(mapped->data());
	const ca::usz base = reinterpret_cast<ca::usz>(mapped->data());
	ca::usz start = base;
	static_cast<void>(ca::align_up(start, alignment));
	const ca::usz head = start - base;
	const ca::usz tail = span - head - size;
	if (head != 0U)
		release(mapped->data(), head);
	if (tail != 0U)
		release(reinterpret_cast<void*>(start + size), tail);
	return reinterpret_cast<void*>(start);
}

[[nodiscard]] static auto copied(const ca::allocation block, const ca::usz size, const ca::usz alignment, const ca::usz page) noexcept -> ca::result<ca::allocation> {

	ca::usz bytes = size;
	if (ca::align_up(bytes, page)) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	auto moved = map_pages(bytes, alignment, page);
	if (!moved) [[unlikely]]
		return moved.propagate();
	ca::copy_bytes(*moved, block.data(), ca::min(block.size(), size));
	ca::usz old = block.size();
	static_cast<void>(ca::align_up(old, page));
	release(block.data(), old);
	return ca::allocation{*moved, bytes};
}

[[nodiscard]] auto ca::page_allocator::allocate(const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

	if (size == 0U) [[unlikely]]
		return ca::allocation::empty(alignment);
	const ca::usz page = page_size();
	ca::usz bytes = size;
	if (ca::align_up(bytes, page)) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	auto mapped = map_pages(bytes, alignment, page);
	if (!mapped) [[unlikely]]
		return mapped.propagate();
	return ca::allocation{*mapped, bytes};
}

[[nodiscard]] auto ca::page_allocator::allocate_zeroed(const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {
	return this->allocate(size, alignment);
}

auto ca::page_allocator::deallocate(const ca::allocation block, const ca::usz) const noexcept -> void {

	if (block.size() == 0U) [[unlikely]]
		return;
	ca::usz bytes = block.size();
	static_cast<void>(ca::align_up(bytes, page_size()));
	release(block.data(), bytes);
}

[[nodiscard]] auto ca::page_allocator::expand(const ca::allocation block, const ca::usz size, const ca::usz) const noexcept -> ca::optional<ca::allocation> {

	if (block.size() == 0U || size == 0U)
		return ca::nullopt;
	const ca::usz page = page_size();
	ca::usz old = block.size();
	ca::usz bytes = size;
	static_cast<void>(ca::align_up(old, page));
	if (ca::align_up(bytes, page)) [[unlikely]]
		return ca::nullopt;
	if (bytes == old)
		return ca::allocation{block.data(), old};
	if (!ca::sys::mremap(pages(block.data(), old), bytes, ca::sys::remapping::in_place))
		return ca::nullopt;
	return ca::allocation{block.data(), bytes};
}

[[nodiscard]] auto ca::page_allocator::reallocate(const ca::allocation block, const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

	if (block.size() == 0U)
		return this->allocate(size, alignment);
	if (size == 0U) {
		this->deallocate(block, alignment);
		return ca::allocation::empty(alignment);
	}
	const ca::usz page = page_size();
	if (alignment > page) {
		auto expanded = this->expand(block, size, alignment);
		if (expanded)
			return *expanded;
		return copied(block, size, alignment, page);
	}
	ca::usz old = block.size();
	ca::usz bytes = size;
	static_cast<void>(ca::align_up(old, page));
	if (ca::align_up(bytes, page)) [[unlikely]]
		return ca::failure{ca::errc::out_of_memory};
	if (bytes == old)
		return ca::allocation{block.data(), old};
	auto moved = ca::sys::mremap(pages(block.data(), old), bytes, ca::sys::remapping::may_move);
	if (moved) [[likely]]
		return ca::allocation{moved->data(), bytes};
	if (moved.error() == ca::errc::not_supported)
		return copied(block, size, alignment, page);
	return moved.propagate();
}
