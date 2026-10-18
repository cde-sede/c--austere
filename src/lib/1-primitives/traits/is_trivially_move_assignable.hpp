#ifndef ca_primitives_traits_is_trivially_move_assignable_hpp
#define ca_primitives_traits_is_trivially_move_assignable_hpp

#include "1-primitives/traits/add_lvalue_reference.hpp"
#include "1-primitives/traits/add_rvalue_reference.hpp"
#include "1-primitives/traits/is_trivially_assignable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS TRIVIALLY MOVE ASSIGNABLE ---------------------------------------

	/* @DOC std::is_trivially_move_assignable_v, as a concept. */
	template <typename T>
	concept is_trivially_move_assignable = ca::is_trivially_assignable<ca::add_lvalue_reference<T>, ca::add_rvalue_reference<T>>;

} // namespace ca

#endif // ca_primitives_traits_is_trivially_move_assignable_hpp
