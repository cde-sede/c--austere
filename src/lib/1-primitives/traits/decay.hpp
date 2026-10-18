#ifndef ca_primitives_traits_decay_hpp
#define ca_primitives_traits_decay_hpp

#include "1-primitives/traits/add_pointer.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_function.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "1-primitives/traits/remove_extent.hpp"
#include "1-primitives/traits/remove_reference.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DECAY --------------------------------------------------------------

	namespace impl {

		template <typename T>
		struct decay final {
				using self = ca::impl::decay<T>;
				using type = ca::remove_cv<T>;
				decay(void) = delete;
		}; // struct decay

		template <typename T>
		requires ca::is_array<T>
		struct decay<T> final {
				using self = ca::impl::decay<T>;
				using type = ca::remove_extent<T>*;
				decay(void) = delete;
		}; // struct decay

		template <typename T>
		requires ca::is_function<T>
		struct decay<T> final {
				using self = ca::impl::decay<T>;
				using type = ca::add_pointer<T>;
				decay(void) = delete;
		}; // struct decay

	} // namespace impl


	/* @DOC std::decay_t. */
	template <typename T>
	using decay = typename ca::impl::decay<ca::remove_reference<T>>::type;

} // namespace ca

#endif // ca_primitives_traits_decay_hpp
