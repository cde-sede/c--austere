#ifndef ca_primitives_traits_add_volatile_hpp
#define ca_primitives_traits_add_volatile_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ADD VOLATILE -------------------------------------------------------

	/* @DOC std::add_volatile_t. */
	template <typename T>
	using add_volatile = volatile T;

} // namespace ca

#endif // ca_primitives_traits_add_volatile_hpp
