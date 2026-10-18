#ifndef ca_lifecycle_memory_arena_allocator_hpp
#define ca_lifecycle_memory_arena_allocator_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/impl/arena_state.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ARENA ALLOCATOR ----------------------------------------------------

	namespace impl {

		[[nodiscard, gnu::noinline]] auto arena_grow(ca::impl::arena_state* state, ca::usz size, ca::usz alignment) noexcept -> ca::result<ca::allocation>;

	} // namespace impl


	/* @DOC Stateful allocator handing out blocks of an arena, one pointer wide; from
	   arena::allocator(). Copies share the arena, which must outlive them. Allocation bumps a
	   pointer; deallocate and expand act on the newest block only, and are no-ops on others.
	   Not thread-safe. */
	class arena_allocator final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::arena_allocator;


			// -- private members --------------------------------------------

			ca::impl::arena_state* _state;


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC Over the state of an arena; arena::allocator() builds it. */
			constexpr explicit arena_allocator(ca::impl::arena_state* const state) noexcept
			: _state{state} {
			}


			// -- methods ----------------------------------------------------

			[[nodiscard, gnu::always_inline]] auto allocate(const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

				const ca::usz end = reinterpret_cast<ca::usz>(_state->_end);
				const ca::usz start = (reinterpret_cast<ca::usz>(_state->_top) + alignment - 1U) & ~(alignment - 1U);
				if (start <= end && size <= end - start) [[likely]] {
					_state->_top = reinterpret_cast<char*>(start + size);
					return ca::allocation{reinterpret_cast<void*>(start), size};
				}
				return ca::impl::arena_grow(_state, size, alignment);
			}

			/* @DOC Gives the room back when block is the newest one; otherwise nothing. */
			[[gnu::always_inline]] auto deallocate(const ca::allocation block, const ca::usz) const noexcept -> void {
				if (static_cast<char*>(block.data()) + block.size() == _state->_top)
					_state->_top = static_cast<char*>(block.data());
			}

			/* @DOC The newest block grows or shrinks while its chunk has room; any block shrinks by
			   keeping its size. */
			[[nodiscard]] auto expand(const ca::allocation block, const ca::usz size, const ca::usz) const noexcept -> ca::optional<ca::allocation> {

				char* const data = static_cast<char*>(block.data());
				if (data + block.size() == _state->_top && size <= static_cast<ca::usz>(_state->_end - data)) {
					_state->_top = data + size;
					return ca::allocation{data, size};
				}
				if (size <= block.size())
					return block;
				return ca::nullopt;
			}

	}; // class arena_allocator

} // namespace ca

#endif // ca_lifecycle_memory_arena_allocator_hpp
