#ifndef ca_vocabulary_impl_view_rules_hpp
#define ca_vocabulary_impl_view_rules_hpp

#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_lvalue_reference.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "1-primitives/traits/remove_cvref.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- VIEW RULES ---------------------------------------------------------

	namespace impl {

		/* @DOC R declares static constexpr bool BORROWED = true: it refers to elements it does
		   not own, so a view made from a temporary R stays valid. */
		template <typename R>
		concept has_borrowed_claim = requires {
			requires ca::remove_cvref<R>::BORROWED;
		};

		/* @DOC A view of T elements may be made from range (an R&&): an lvalue, a borrowed view, or
		   const T, which may bind a temporary for the length of a call. */
		template <typename R, typename T>
		concept is_viewable = ca::is_lvalue_reference<R> || ca::is_const<T> || ca::impl::has_borrowed_claim<R>;

		/* @DOC Arrays of const code units: text literals, whose last unit is the NUL. Views take
		   them through ca::sv, never whole. */
		template <typename U>
		concept is_const_text = ca::is_const<U> && ca::is_character<ca::remove_cv<U>>;

	} // namespace impl

} // namespace ca

#endif // ca_vocabulary_impl_view_rules_hpp
