#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/fill_bytes.hpp"
#include "1-primitives/numeric/max.hpp"
#include "1-primitives/numeric/min.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "3-system/thread/thread_key.hpp"
#include "3-system/thread/thread_key_create.hpp"
#include "3-system/thread/thread_key_set.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/page_allocator.hpp"


static constexpr ca::u32 spare_limit   = 2U;
static constexpr ca::usz carve_bytes   = 4096U;
static constexpr ca::usz pool_chunk    = 65536U;
static constexpr ca::u8  poison_byte   = 0xDFU;

[[nodiscard]] static consteval auto empty_queues(void) noexcept -> ca::impl::heap_queues {

	ca::impl::heap_queues queues{};
	for (ca::impl::heap_span*& span : queues._spans)
		span = &ca::impl::heap_empty_span;
	return queues;
}



// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HEAP STATE ---------------------------------------------------------

	namespace impl {

		constinit ca::impl::heap_span heap_empty_span{};

		constinit ca::impl::heap_state heap_empty_state{empty_queues()};

		constinit thread_local ca::impl::heap_state* heap_current = &ca::impl::heap_empty_state;

	} // namespace impl

} // namespace ca


static constinit ca::atomic<ca::u32> pool_lock{0U}; // file-state: ignore
static constinit ca::impl::heap_state* idle_heaps = nullptr; // file-state: ignore
static constinit char* pool_next = nullptr; // file-state: ignore
static constinit char* pool_end = nullptr; // file-state: ignore
static constinit bool key_ready = false; // file-state: ignore
static constinit ca::usz key_id = 0U; // file-state: ignore


// -- SIZES ------------------------------------------------------------------

[[nodiscard]] static auto is_small(const ca::usz size, const ca::usz alignment) noexcept -> bool {
	return size <= ca::impl::heap_largest_class && alignment <= ca::impl::heap_largest_class;
}

[[nodiscard]] static auto class_for(const ca::usz size, const ca::usz alignment) noexcept -> ca::u32 {

	ca::u32 index = ca::impl::heap_class_of(size);
	if (alignment > ca::heap::ALIGNMENT) [[unlikely]]
		while ((ca::impl::heap_class_size(index) & (alignment - 1U)) != 0U)
			++index;
	return index;
}

[[nodiscard]] static auto full_mark(void) noexcept -> void* {
	return reinterpret_cast<void*>(ca::usz{1U});
}

[[nodiscard]] static auto next_of(void* const block) noexcept -> void*& {
	return *static_cast<void**>(block);
}


// -- POOL -------------------------------------------------------------------

static auto lock(void) noexcept -> void {
	while (pool_lock.exchange<ca::memory_order::acquire>(1U) != 0U)
		while (pool_lock.load<ca::memory_order::relaxed>() != 0U) {
		}
}

static auto unlock(void) noexcept -> void {
	pool_lock.store<ca::memory_order::release>(0U);
}


// -- SPANS ------------------------------------------------------------------

[[nodiscard]] static auto base_of(ca::impl::heap_span* const span) noexcept -> char* {
	return reinterpret_cast<char*>(span) - ca::impl::heap_span_header;
}

static auto link_front(ca::impl::heap_state* const heap, ca::impl::heap_span* const span) noexcept -> void {

	ca::impl::heap_span*& first = heap->_queues._spans[span->_class];
	span->_previous = nullptr;
	    span->_next = first == &ca::impl::heap_empty_span ? nullptr : first;
	if (span->_next != nullptr)
		span->_next->_previous = span;
	first = span;
}

static auto unlink(ca::impl::heap_state* const heap, ca::impl::heap_span* const span) noexcept -> void {

	if (span->_previous != nullptr)
		span->_previous->_next = span->_next;
	else
		heap->_queues._spans[span->_class] = span->_next != nullptr ? span->_next : &ca::impl::heap_empty_span;
	if (span->_next != nullptr)
		span->_next->_previous = span->_previous;
	    span->_next = nullptr;
	span->_previous = nullptr;
}

static auto release(ca::impl::heap_state* const heap, ca::impl::heap_span* const span) noexcept -> void {

	if (heap->_spare_count < spare_limit) {
		 span->_next = heap->_spare;
		heap->_spare = span;
		++heap->_spare_count;
		return;
	}
	ca::page_allocator{}.deallocate(ca::allocation{base_of(span), ca::impl::heap_span_size}, ca::impl::heap_span_size);
}

[[nodiscard]] static auto fresh_span(ca::impl::heap_state* const heap, const ca::u32 index) noexcept -> ca::result<ca::impl::heap_span*> {

	char* base = nullptr;
	if (heap->_spare != nullptr) {
		        base = base_of(heap->_spare);
		heap->_spare = heap->_spare->_next;
		--heap->_spare_count;
	}
	else {
		auto mapped = ca::page_allocator{}.allocate(ca::impl::heap_span_size, ca::impl::heap_span_size);
		if (!mapped) [[unlikely]]
			return mapped.propagate();
		base = static_cast<char*>(mapped->data());
	}
	ca::impl::heap_span* const span = ca::construct_at(reinterpret_cast<ca::impl::heap_span*>(base + ca::impl::heap_span_header));
	const ca::usz size = ca::impl::heap_class_size(index);
	 span->_class = index;
	  span->_size = size;
	span->_unused = base;
	 span->_limit = base + (ca::impl::heap_span_header / size) * size;
	  span->_heap = heap;
	return span;
}

static auto adopt(ca::impl::heap_span* const span, void* const list) noexcept -> void {

	ca::u32 count = 1U;
	void* tail = list;
	while (next_of(tail) != nullptr) {
		tail = next_of(tail);
		++count;
	}
	next_of(tail) = span->_free;
	span->_free = list;
	span->_used -= count;
}

[[nodiscard]] static auto carve(ca::impl::heap_span* const span) noexcept -> bool {

	char* const start = span->_unused;
	if (start == span->_limit)
		return false;
	const ca::usz size = span->_size;
	ca::usz bytes = ca::max(size, carve_bytes);
	bytes -= bytes % size;
	bytes = ca::min(bytes, static_cast<ca::usz>(span->_limit - start));
	char* const last = start + bytes - size;
	for (char* block = start; block != last; block += size)
		next_of(block) = block + size;
	next_of(last) = nullptr;
	  span->_free = start;
	span->_unused = start + bytes;
	return true;
}

static auto local_free(ca::impl::heap_span* const span, void* const block) noexcept -> void {

	void* const head = span->_free;
	next_of(block) = head;
	span->_free = block;
	--span->_used;
	if (head == nullptr || (span->_used == 0U && span->_heap->_queues._spans[span->_class] != span))
		ca::impl::heap_settle(span);
}

static auto remote_free(ca::impl::heap_span* const span, void* const block) noexcept -> void {

	void* old = span->_remote.load<ca::memory_order::relaxed>();
	for (;;) {
		if (old == full_mark()) {
			ca::impl::heap_state* const heap = span->_heap;
			void* head = heap->_pending.load<ca::memory_order::relaxed>();
			do
				next_of(block) = head;
			while (!heap->_pending.compare_exchange_weak<ca::memory_order::release, ca::memory_order::relaxed>(head, block));
			return;
		}
		next_of(block) = old;
		if (span->_remote.compare_exchange_weak<ca::memory_order::release, ca::memory_order::relaxed>(old, block))
			return;
	}
}

static auto take_pending(ca::impl::heap_state* const heap) noexcept -> void {

	void* block = heap->_pending.exchange<ca::memory_order::acquire>(nullptr);
	while (block != nullptr) {
		void* const next = next_of(block);
		local_free(ca::impl::heap_span_of(block), block);
		block = next;
	}
}

[[nodiscard]] static auto allocate_from(ca::impl::heap_state* const heap, const ca::u32 index) noexcept -> ca::result<ca::allocation> {
	for (;;) {
		ca::impl::heap_span* const span = heap->_queues._spans[index];
		if (span != &ca::impl::heap_empty_span) {
			if (span->_free == nullptr) {
				void* const remote = span->_remote.exchange<ca::memory_order::acquire>(nullptr);
				if (remote != nullptr)
					adopt(span, remote);
				else if (!carve(span)) {
					void* expected = nullptr;
					if (span->_remote.compare_exchange_strong<ca::memory_order::acquire>(expected, full_mark())) {
						unlink(heap, span);
						continue;
					}
					adopt(span, span->_remote.exchange<ca::memory_order::acquire>(nullptr));
				}
			}
			void* const block = span->_free;
			span->_free = next_of(block);
			++span->_used;
			return ca::allocation{block, span->_size};
		}
		if (heap->_pending.load<ca::memory_order::relaxed>() != nullptr) {
			take_pending(heap);
			continue;
		}
		auto fresh = fresh_span(heap, index);
		if (!fresh) [[unlikely]]
			return fresh.propagate();
		link_front(heap, *fresh);
	}
}


// -- THREADS ----------------------------------------------------------------

static auto abandon(void* const raw) noexcept -> void {

	ca::impl::heap_state* const heap = static_cast<ca::impl::heap_state*>(raw);
	take_pending(heap);
	for (ca::impl::heap_span* span : heap->_queues._spans)
		while (span != &ca::impl::heap_empty_span && span != nullptr) {
			ca::impl::heap_span* const next = span->_next;
			if (span->_used == 0U) {
				unlink(heap, span);
				release(heap, span);
			}
			span = next;
		}
	ca::impl::heap_current = &ca::impl::heap_empty_state;
	lock();
	heap->_next_idle = idle_heaps;
	      idle_heaps = heap;
	unlock();
}

[[nodiscard]] static auto acquire(void) noexcept -> ca::result<ca::impl::heap_state*> {

	lock();
	ca::impl::heap_state* heap = idle_heaps;
	if (heap != nullptr)
		idle_heaps = heap->_next_idle;
	else {
		if (static_cast<ca::usz>(pool_end - pool_next) < sizeof(ca::impl::heap_state)) {
			auto chunk = ca::page_allocator{}.allocate(pool_chunk, alignof(ca::impl::heap_state));
			if (!chunk) [[unlikely]] {
				unlock();
				return chunk.propagate();
			}
			pool_next = static_cast<char*>(chunk->data());
			 pool_end = pool_next + chunk->size();
		}
		heap = ca::construct_at(reinterpret_cast<ca::impl::heap_state*>(pool_next), empty_queues());
		pool_next += sizeof(ca::impl::heap_state);
	}
	if (!key_ready) {
		auto key = ca::sys::thread_key_create(&abandon, ca::sys::exit_stage::memory);
		if (key) {
			   key_id = key->id();
			key_ready = true;
		}
	}
	const bool registered = key_ready;
	const ca::usz id = key_id;
	unlock();
	if (registered)
		static_cast<void>(ca::sys::thread_key_set(ca::sys::thread_key{id}, heap));
	ca::impl::heap_current = heap;
	return heap;
}


// -- CHECKS -----------------------------------------------------------------

[[noreturn, gnu::noinline, gnu::cold]] static auto block_freed_with_another_size_class(void) noexcept -> void {
	ca::trap();
}

[[noreturn, gnu::noinline, gnu::cold]] static auto pointer_freed_is_no_heap_block(void) noexcept -> void {
	ca::trap();
}


// -- HEAP -------------------------------------------------------------------

[[nodiscard, gnu::noinline]] auto ca::impl::heap_allocate(const ca::usz size, const ca::usz alignment) noexcept -> ca::result<ca::allocation> {

	if (size == 0U) [[unlikely]]
		return ca::allocation::empty(alignment);
	if (!is_small(size, alignment))
		return ca::page_allocator{}.allocate(size, alignment);
	ca::impl::heap_state* heap = ca::impl::heap_current;
	if (heap == &ca::impl::heap_empty_state) [[unlikely]] {
		auto acquired = acquire();
		if (!acquired) [[unlikely]]
			return acquired.propagate();
		heap = *acquired;
	}
	return allocate_from(heap, class_for(size, alignment));
}

[[gnu::noinline]] auto ca::impl::heap_deallocate(const ca::allocation block, const ca::usz alignment) noexcept -> void {

	if (block.size() == 0U) [[unlikely]]
		return;
	if (!is_small(block.size(), alignment)) {
		ca::page_allocator{}.deallocate(block, alignment);
		return;
	}
	ca::impl::heap_span* const span = ca::impl::heap_span_of(block.data());
	if constexpr (ca_debug != 0)
		ca::impl::heap_check(span, block, alignment);
	if (span->_heap == ca::impl::heap_current)
		local_free(span, block.data());
	else
		remote_free(span, block.data());
}

[[gnu::noinline]] auto ca::impl::heap_settle(ca::impl::heap_span* const span) noexcept -> void {

	ca::impl::heap_state* const heap = span->_heap;
	if (span->_remote.load<ca::memory_order::relaxed>() == full_mark()) {
		span->_remote.store<ca::memory_order::relaxed>(nullptr);
		link_front(heap, span);
		return;
	}
	if (span->_used == 0U && heap->_queues._spans[span->_class] != span) {
		unlink(heap, span);
		release(heap, span);
	}
}

auto ca::impl::heap_check(const ca::impl::heap_span* const span, const ca::allocation block, const ca::usz alignment) noexcept -> void {

	char* const data = static_cast<char*>(block.data());
	const char* const base = reinterpret_cast<const char*>(span) - ca::impl::heap_span_header;
	if (data >= span->_limit || static_cast<ca::usz>(data - base) % span->_size != 0U) [[unlikely]]
		pointer_freed_is_no_heap_block();
	if (span->_class != class_for(block.size(), alignment)) [[unlikely]]
		block_freed_with_another_size_class();
	ca::fill_bytes(data + sizeof(void*), poison_byte, span->_size - sizeof(void*));
}

[[nodiscard]] auto ca::heap::allocate_zeroed(const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

	if (!is_small(size, alignment))
		return ca::page_allocator{}.allocate_zeroed(size, alignment);
	auto block = this->allocate(size, alignment);
	if (block) [[likely]]
		ca::fill_bytes(block->data(), 0U, block->size());
	return block;
}

[[nodiscard]] auto ca::heap::expand(const ca::allocation block, const ca::usz size, const ca::usz alignment) const noexcept -> ca::optional<ca::allocation> {

	if (block.size() == 0U || size == 0U)
		return ca::nullopt;
	const bool was_small = is_small(block.size(), alignment);
	const bool stays_small = is_small(size, alignment);
	if (!was_small && !stays_small)
		return ca::page_allocator{}.expand(block, size, alignment);
	if (was_small && stays_small) {
		const ca::impl::heap_span* const span = ca::impl::heap_span_of(block.data());
		if (class_for(size, alignment) == span->_class)
			return ca::allocation{block.data(), span->_size};
	}
	return ca::nullopt;
}

[[nodiscard]] auto ca::heap::reallocate(const ca::allocation block, const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

	if (block.size() == 0U)
		return this->allocate(size, alignment);
	if (size == 0U) {
		this->deallocate(block, alignment);
		return ca::allocation::empty(alignment);
	}
	if (!is_small(block.size(), alignment) && !is_small(size, alignment))
		return ca::page_allocator{}.reallocate(block, size, alignment);
	auto in_place = this->expand(block, size, alignment);
	if (in_place)
		return *in_place;
	auto moved = this->allocate(size, alignment);
	if (!moved) [[unlikely]]
		return moved;
	ca::copy_bytes(moved->data(), block.data(), ca::min(block.size(), size));
	this->deallocate(block, alignment);
	return moved;
}
