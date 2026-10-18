#ifndef ca_primitives_traits_reference_constructs_from_temporary_hpp
#define ca_primitives_traits_reference_constructs_from_temporary_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REFERENCE CONSTRUCTS FROM TEMPORARY --------------------------------

	/* @DOC std::reference_constructs_from_temporary_v, as a concept. */
	template <typename T, typename U> // concept-name: ignore
	concept reference_constructs_from_temporary = __reference_constructs_from_temporary(T, U);

} // namespace ca

#endif // ca_primitives_traits_reference_constructs_from_temporary_hpp
