#ifndef ca_lifecycle_memory_impl_heap_span_hpp
#define ca_lifecycle_memory_impl_heap_span_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "4-lifecycle/memory/impl/heap_classes.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- HEAP SPAN ----------------------------------------------------------

	namespace impl {

		struct heap_state;


		/* @DOC Header of a heap span: heap_span_size bytes aligned to their size, blocks of one class
		   from the start, this header in the last bytes, so a block finds it by masking its address.
		   The first line is the owner's alone. The second takes frees from other threads: a list
		   of blocks, or heap_span_full once the owner found nothing left, frees then going to the
		   owner's heap instead. A thread freeing a block touches the span no more after it. */
		struct alignas(64) heap_span final {
				using self = ca::impl::heap_span;

				void*                         _free;
				ca::u32                       _used;
				ca::u32                       _class;
				ca::usz                       _size;
				char*                         _unused;
				char*                         _limit;
				ca::impl::heap_state*         _heap;
				self*                         _next;
				self*                         _previous;
				alignas(64) ca::atomic<void*> _remote;
		}; // struct heap_span

		static_assert(sizeof(ca::impl::heap_span) == 128U);

		inline constexpr ca::usz heap_span_header = ca::impl::heap_span_size - sizeof(ca::impl::heap_span);

		/* @DOC Header of the span holding block, a small heap block. */
		[[nodiscard, gnu::always_inline]] inline auto heap_span_of(void* const block) noexcept -> ca::impl::heap_span* {

			const ca::usz base = reinterpret_cast<ca::usz>(block) & ~(ca::impl::heap_span_size - 1U);
			return reinterpret_cast<ca::impl::heap_span*>(base + ca::impl::heap_span_header);
		}

	} // namespace impl

} // namespace ca

#endif // ca_lifecycle_memory_impl_heap_span_hpp
