#ifndef ca_primitives_traits_is_virtual_base_of_hpp
#define ca_primitives_traits_is_virtual_base_of_hpp

#include "1-primitives/config/has_builtin.hpp"
#include "1-primitives/traits/is_base_of.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS VIRTUAL BASE OF -------------------------------------------------

	/* @DOC std::is_virtual_base_of_v, as a concept. Without __builtin_is_virtual_base_of: private or
	   protected virtual bases report false. */
#if ca_has_builtin(__builtin_is_virtual_base_of)
	template <typename B, typename D>
	concept is_virtual_base_of = __builtin_is_virtual_base_of(B, D);
#else
	namespace impl {

		template <typename B, typename D>
		concept is_virtual_base = ca::is_convertible<D*, B*> && !requires(B* b) {
			static_cast<D*>(b);
		};

	} // namespace impl


	template <typename B, typename D>
	concept is_virtual_base_of = ca::is_base_of<B, D> && !ca::is_same<ca::remove_cv<B>, ca::remove_cv<D>>
		&& ca::impl::is_virtual_base<ca::remove_cv<B>, ca::remove_cv<D>>;
#endif

} // namespace ca

#endif // ca_primitives_traits_is_virtual_base_of_hpp
