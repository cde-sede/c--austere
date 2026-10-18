#ifndef ca_lifecycle_memory_impl_heap_state_hpp
#define ca_lifecycle_memory_impl_heap_state_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/u32.hpp"
#include "4-lifecycle/memory/impl/heap_classes.hpp"
#include "4-lifecycle/memory/impl/heap_span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HEAP STATE ---------------------------------------------------------

	namespace impl {

		/* @DOC Per class, the first span with room left (or heap_empty_span), linked to the others. */
		struct heap_queues final {
				using self = ca::impl::heap_queues;

				ca::impl::heap_span* _spans[ca::impl::heap_class_count];
		}; // struct heap_queues


		/* @DOC A thread's heap. Spans whose blocks are all out leave their queue until one comes
		   back. Heaps are never freed: an exiting thread returns its heap to a pool, the next new
		   thread adopts it with its spans. _pending gathers blocks other threads freed into full
		   spans. */
		struct heap_state final {
				using self = ca::impl::heap_state;

				ca::impl::heap_queues         _queues;
				ca::impl::heap_span*          _spare;
				ca::u32                       _spare_count;
				self*                         _next_idle;
				alignas(64) ca::atomic<void*> _pending;

				constexpr explicit heap_state(const ca::impl::heap_queues queues) noexcept
				: _queues{queues}, _spare{nullptr}, _spare_count{0U}, _next_idle{nullptr}, _pending{nullptr} {
				}
		}; // struct heap_state


		/* @DOC Span with nothing to give, queued in every empty class. */
		extern constinit ca::impl::heap_span heap_empty_span;

		/* @DOC Heap of threads that have not allocated yet. Owns no span. */
		extern constinit ca::impl::heap_state heap_empty_state;

		/* @DOC The calling thread's heap. */
		extern constinit thread_local ca::impl::heap_state* heap_current;

	} // namespace impl

} // namespace ca

#endif // ca_lifecycle_memory_impl_heap_state_hpp
