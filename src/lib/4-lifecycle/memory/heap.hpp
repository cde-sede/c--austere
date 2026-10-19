/* @DOC Performance of ca::heap.

   Layout: per thread, a heap of 48 size-class queues of 1 MiB spans aligned to their size;
   each span serves one class from its start, its 128-byte header in the last bytes. A block
   finds its span by masking its address: no per-block header, and deallocate needs the size
   only to tell small from large (past 128 KiB, page_allocator).

   | Operation | Time | Notes |
   |---|---|---|
   | allocate, small, block on the free list | O(1) | ~10 instructions inline: TLS load, class from size (shift, or bit_width for >128), pop |
   | deallocate, small, same thread | O(1) | ~15 instructions inline: mask, owner compare, push |
   | allocate, list empty | O(1) amortized | out of line: take remote frees, else carve a page of fresh blocks, else next span or a new one |
   | deallocate, other thread | O(1) | one CAS on the span's list, or on its heap's when the span is full |
   | allocate, deallocate, large | one syscall | mmap / munmap |
   | reallocate | O(1) within a class or large to large | large to large is mremap: no byte copied |

   Space: classes 16 to 128 by 16, then four per doubling, at most 25% over the size asked
   for; blocks are carved a page at a time, so untouched spans cost address space only. An
   emptied span goes back to the system unless it is its class's first; two per heap are kept
   for reuse.

   Threads: no lock and no atomic on a thread's own blocks. A block freed by another thread
   goes to its span's atomic list, collected when the owner runs out; a span marked full sends
   such frees to its heap instead, so that the freeing thread never touches a span the owner may
   release. Heaps are never freed: an exiting thread's heap (pthread key destructor) goes to a
   pool under a spin lock, the next new thread adopts it with its spans. A fork while another
   thread holds that lock leaves the child unable to start a new heap.

   Measurements: see dynamic_array and path; against glibc malloc, 1.25x to 2.5x faster on
   allocation-heavy cases, equal elsewhere. */

#ifndef ca_lifecycle_memory_heap_hpp
#define ca_lifecycle_memory_heap_hpp

#include "1-primitives/config/debug.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/impl/heap_classes.hpp"
#include "4-lifecycle/memory/impl/heap_span.hpp"
#include "4-lifecycle/memory/impl/heap_state.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HEAP ---------------------------------------------------------------

	namespace impl {

		[[nodiscard, gnu::noinline]] auto heap_allocate(ca::usz size, ca::usz alignment) noexcept -> ca::result<ca::allocation>;

		[[gnu::noinline]] auto heap_deallocate(ca::allocation block, ca::usz alignment) noexcept -> void;

		[[gnu::noinline]] auto heap_settle(ca::impl::heap_span* span) noexcept -> void;

		auto heap_check(const ca::impl::heap_span* span, ca::allocation block, ca::usz alignment) noexcept -> void;

	} // namespace impl


	/* @DOC The general-purpose, stateless allocator, default of allocator-aware types. Sizes up to
	   128 KiB come from size classes (16 to 128 by 16, then four per doubling), each thread
	   serving its own spans of 1 MiB with no lock and no atomic; larger ones from page_allocator.
	   Any thread may free any block: a block freed away from its thread goes back through one
	   atomic exchange. Usable size is the class size. Debug builds check that a block goes back
	   with its size class and alignment, and poison it. */
	class heap final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::heap;


		public:

			// -- public constants -------------------------------------------

			static constexpr bool STATELESS = true;

			static constexpr ca::usz ALIGNMENT = 16U;


			// -- public lifecycle -------------------------------------------

			constexpr heap(void) noexcept = default;


			// -- methods ----------------------------------------------------

			[[nodiscard, gnu::always_inline]] auto allocate(const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

				if (size - 1U < ca::impl::heap_largest_class && alignment <= self::ALIGNMENT) [[likely]] {
					ca::impl::heap_span* const span = ca::impl::heap_current->_queues._spans[ca::impl::heap_class_of(size)];
					void* const block = span->_free;
					if (block != nullptr) [[likely]] {
						span->_free = *static_cast<void**>(block);
						++span->_used;
						return ca::allocation{block, span->_size};
					}
				}
				return ca::impl::heap_allocate(size, alignment);
			}

			[[gnu::always_inline]] auto deallocate(const ca::allocation block, const ca::usz alignment) const noexcept -> void {

				if (block.size() - 1U < ca::impl::heap_largest_class && alignment <= self::ALIGNMENT) [[likely]] {
					ca::impl::heap_span* const span = ca::impl::heap_span_of(block.data());
					ca::impl::heap_state* const owner = ca::impl::heap_current;
					if (span->_heap == owner) [[likely]] {
						if constexpr (ca_debug != 0)
							ca::impl::heap_check(span, block, alignment);
						void* const head = span->_free;
						*static_cast<void**>(block.data()) = head;
						span->_free = block.data();
						--span->_used;
						if (head == nullptr || (span->_used == 0U && owner->_queues._spans[span->_class] != span)) [[unlikely]]
							ca::impl::heap_settle(span);
						return;
					}
				}
				ca::impl::heap_deallocate(block, alignment);
			}

			/* @DOC allocate, zeroed: large blocks come as fresh pages, small ones are filled. */
			[[nodiscard]] auto allocate_zeroed(ca::usz size, ca::usz alignment) const noexcept -> ca::result<ca::allocation>;

			/* @DOC In place when size keeps the block's class, or both sizes are large and the pages
			   remap where they are. */
			[[nodiscard]] auto expand(ca::allocation block, ca::usz size, ca::usz alignment) const noexcept -> ca::optional<ca::allocation>;

			/* @DOC In place when size keeps the class; large to large remaps pages without copying;
			   otherwise a new block and a copy. */
			[[nodiscard]] auto reallocate(ca::allocation block, ca::usz size, ca::usz alignment) const noexcept -> ca::result<ca::allocation>;

	}; // class heap

} // namespace ca

#endif // ca_lifecycle_memory_heap_hpp
