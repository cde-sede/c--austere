#ifndef ca_lifecycle_memory_allocation_hpp
#define ca_lifecycle_memory_allocation_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALLOCATION ---------------------------------------------------------

	/* @DOC A block from an allocator: its address and its usable size, at least the size asked for.
	   Handed back whole to deallocate, with any size between the one asked for and size(). A
	   zero-size allocation holds a non-null address and needs no deallocate. */
	class allocation final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::allocation;


			// -- private members --------------------------------------------

			void*   _data;
			ca::usz _size;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr allocation(void* const data, const ca::usz size) noexcept
			: _data{data}, _size{size} {
			}

			/* @DOC The zero-size allocation for alignment: address alignment, nothing reserved. */
			[[nodiscard]] static auto empty(const ca::usz alignment) noexcept -> self {
				return self{reinterpret_cast<void*>(alignment), 0U};
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto data(void) const noexcept -> void* {
				return _data;
			}

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _size;
			}

			/* @DOC A copy reporting size instead, for handing back part of a block. */
			[[nodiscard]] constexpr auto with_size(const ca::usz size) const noexcept -> self {
				return self{_data, size};
			}

	}; // class allocation

} // namespace ca

#endif // ca_lifecycle_memory_allocation_hpp
