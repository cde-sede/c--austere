#ifndef ca_lifecycle_memory_libc_allocator_hpp
#define ca_lifecycle_memory_libc_allocator_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/memory/free.hpp"
#include "3-system/memory/malloc_alignment.hpp"
#include "3-system/memory/mallocarray.hpp"
#include "3-system/memory/reallocarray.hpp"
#include "4-lifecycle/memory/allocation.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LIBC ALLOCATOR -----------------------------------------------------

	/* @DOC Stateless allocator over the C library's malloc, realloc and free (through ca::sys):
	   the baseline benchmarks compare against. Usable size is the size asked for. Alignments past
	   sys::malloc_alignment fail with errc::invalid_argument. */
	class libc_allocator final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::libc_allocator;


		public:

			// -- public constants -------------------------------------------

			static constexpr bool STATELESS = true;

			static constexpr ca::usz ALIGNMENT = ca::sys::malloc_alignment;


			// -- public lifecycle -------------------------------------------

			constexpr libc_allocator(void) noexcept = default;


			// -- methods ----------------------------------------------------

			[[nodiscard]] auto allocate(const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

				if (alignment > self::ALIGNMENT) [[unlikely]]
					return ca::failure{ca::errc::invalid_argument};
				if (size == 0U) [[unlikely]]
					return ca::allocation::empty(alignment);
				return ca::sys::mallocarray(1U, size).transform([size](void* const block) {
					return ca::allocation{block, size};
				});
			}

			auto deallocate(const ca::allocation block, const ca::usz) const noexcept -> void {
				if (block.size() != 0U)
					ca::sys::free(block.data());
			}

			/* @DOC realloc: grows in place when the C library can, else moves the bytes. */
			[[nodiscard]] auto reallocate(const ca::allocation block, const ca::usz size, const ca::usz alignment) const noexcept -> ca::result<ca::allocation> {

				if (block.size() == 0U)
					return this->allocate(size, alignment);
				if (size == 0U) {
					this->deallocate(block, alignment);
					return ca::allocation::empty(alignment);
				}
				return ca::sys::reallocarray(block.data(), 1U, size).transform([size](void* const resized) {
					return ca::allocation{resized, size};
				});
			}

	}; // class libc_allocator

} // namespace ca

#endif // ca_lifecycle_memory_libc_allocator_hpp
