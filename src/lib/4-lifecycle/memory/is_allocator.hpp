#ifndef ca_lifecycle_memory_is_allocator_hpp
#define ca_lifecycle_memory_is_allocator_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_empty.hpp"
#include "1-primitives/traits/is_nothrow_copy_assignable.hpp"
#include "1-primitives/traits/is_nothrow_copy_constructible.hpp"
#include "1-primitives/traits/is_nothrow_default_constructible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/allocation.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS ALLOCATOR -------------------------------------------------------

	namespace impl {

		/* @DOC A declares static constexpr bool STATELESS = true. */
		template <typename A>
		concept has_stateless_claim = requires {
			requires A::STATELESS;
		};

	} // namespace impl


	/* @DOC Cheap, noexcept-copyable values that hand out memory; what allocator-aware types store.
	   - allocate(size, alignment) const -> result<allocation>: alignment a power of two; size 0
	     succeeds without reserving anything.
	   - deallocate(block, alignment) const -> void: block from allocate (or a resize of it) by
	     this allocator or a copy, alignment as given then. Never fails.
	   Optional, found by ca::expand, ca::reallocate, ca::allocate_zeroed: expand(block, size,
	   alignment) -> optional<allocation> grows or shrinks in place; reallocate(block, size,
	   alignment) -> result<allocation> may move the bytes, block untouched on failure;
	   allocate_zeroed(size, alignment) -> result<allocation>. ALIGNMENT, when declared, is the
	   alignment every block has. A type claiming STATELESS must be empty and noexcept
	   default-constructible, or it is no allocator. */
	template <typename A>
	concept is_allocator = ca::is_nothrow_copy_constructible<A> && ca::is_nothrow_copy_assignable<A>
		&& requires(const A& allocator, const ca::allocation block, const ca::usz size, const ca::usz alignment) {
			{ allocator.allocate(size, alignment) } -> ca::is_same<ca::result<ca::allocation>>;
			{ allocator.deallocate(block, alignment) } -> ca::is_same<void>;
		}
		&& (!ca::impl::has_stateless_claim<A> || (ca::is_empty<A> && ca::is_nothrow_default_constructible<A>));

} // namespace ca

#endif // ca_lifecycle_memory_is_allocator_hpp
