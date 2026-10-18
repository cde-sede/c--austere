#ifndef ca_lifecycle_memory_allocator_of_hpp
#define ca_lifecycle_memory_allocator_of_hpp

#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/declval.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALLOCATOR OF -------------------------------------------------------

	namespace impl {

		template <typename R>
		struct allocator_of_bare final {
				using self = ca::impl::allocator_of_bare<R>;
				using type = decltype(ca::declval<R&>().allocator());
		}; // struct allocator_of_bare

		template <typename R>
		requires ca::is_allocator<R>
		struct allocator_of_bare<R> final {
				using self = ca::impl::allocator_of_bare<R>;
				using type = R;
		}; // struct allocator_of_bare

		template <typename R>
		struct allocator_of final {
				using self = ca::impl::allocator_of<R>;
				using type = typename ca::impl::allocator_of_bare<ca::remove_cvref<R>>::type;
		}; // struct allocator_of

	} // namespace impl


	/* @DOC The allocator for a source R (see is_allocator_source): R itself when it is an
	   allocator, what R::allocator() returns when it is a resource. */
	template <typename R>
	using allocator_of = typename ca::impl::allocator_of<R>::type;

} // namespace ca

#endif // ca_lifecycle_memory_allocator_of_hpp
