#ifndef ca_primitives_traits_is_constant_evaluated_hpp
#define ca_primitives_traits_is_constant_evaluated_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CONSTANT EVALUATED ----------------------------------------------

	/* @DOC std::is_constant_evaluated. */
	constexpr auto is_constant_evaluated(void) noexcept -> bool {
		return __builtin_is_constant_evaluated();
	}

} // namespace ca

#endif // ca_primitives_traits_is_constant_evaluated_hpp
