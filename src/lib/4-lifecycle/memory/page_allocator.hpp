#ifndef ca_lifecycle_memory_page_allocator_hpp
#define ca_lifecycle_memory_page_allocator_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/allocation.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PAGE ALLOCATOR -----------------------------------------------------

	/* @DOC Stateless allocator mapping fresh pages for every block: sizes round up to whole pages,
	   contents start zeroed, deallocate unmaps. Alignments past a page cost a larger mapping,
	   trimmed. reallocate and expand remap the pages (Linux mremap): growing never copies. For
	   large blocks; small ones belong to heap. */
	class page_allocator final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::page_allocator;


		public:

			// -- public constants -------------------------------------------

			static constexpr bool STATELESS = true;

			/* @DOC Smallest page size of every supported target. */
			static constexpr ca::usz ALIGNMENT = 4096U;


			// -- public lifecycle -------------------------------------------

			constexpr page_allocator(void) noexcept = default;


			// -- methods ----------------------------------------------------

			/* @DOC size bytes rounded up to whole pages, zeroed. */
			[[nodiscard]] auto allocate(ca::usz size, ca::usz alignment) const noexcept -> ca::result<ca::allocation>;

			/* @DOC allocate: fresh pages are zero already. */
			[[nodiscard]] auto allocate_zeroed(ca::usz size, ca::usz alignment) const noexcept -> ca::result<ca::allocation>;

			/* @DOC Unmaps the pages. When the kernel cannot (too many mappings to split one), drops their
			   contents instead, keeping the address range. */
			auto deallocate(ca::allocation block, ca::usz alignment) const noexcept -> void;

			/* @DOC Grows or shrinks the mapping where it is; nullopt when the pages after it are taken,
			   for size 0, and off Linux. */
			[[nodiscard]] auto expand(ca::allocation block, ca::usz size, ca::usz alignment) const noexcept -> ca::optional<ca::allocation>;

			/* @DOC Remaps the pages, moving them when needed without copying a byte; copies only for
			   alignments past a page or off Linux. Size 0 deallocates. */
			[[nodiscard]] auto reallocate(ca::allocation block, ca::usz size, ca::usz alignment) const noexcept -> ca::result<ca::allocation>;

	}; // class page_allocator

} // namespace ca

#endif // ca_lifecycle_memory_page_allocator_hpp
