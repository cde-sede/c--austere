#ifndef ca_primitives_traits_negation_hpp
#define ca_primitives_traits_negation_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- NEGATION -----------------------------------------------------------

	/* @DOC std::negation_v, as a concept. Over a type with ::value. */
	template <typename B> // concept-name: ignore
	concept negation = !static_cast<bool>(B::value);

} // namespace ca

#endif // ca_primitives_traits_negation_hpp
