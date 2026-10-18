#ifndef ca_lifecycle_memory_impl_arena_state_hpp
#define ca_lifecycle_memory_impl_arena_state_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ARENA STATE --------------------------------------------------------

	namespace impl {

		/* @DOC Header of every arena chunk, at its start: the chunk before it and its size. */
		struct arena_chunk final {
				using self = ca::impl::arena_chunk;

				self*   _previous;
				ca::usz _size;
		}; // struct arena_chunk


		/* @DOC An arena, kept in its first chunk after the chunk header so that it never moves:
		   blocks go from _top up to _end in the newest chunk. */
		struct arena_state final {
				using self = ca::impl::arena_state;

				char*                  _top;
				char*                  _end;
				ca::impl::arena_chunk* _chunk;
				ca::impl::arena_chunk* _first;
				ca::usz                _chunk_size;
		}; // struct arena_state

	} // namespace impl

} // namespace ca

#endif // ca_lifecycle_memory_impl_arena_state_hpp
