#ifndef ca_primitives_utility_type_key_hpp
#define ca_primitives_utility_type_key_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TYPE KEY -----------------------------------------------------------

	namespace impl {

		template <typename T>
		inline constexpr char type_key_of{};

		/* @DOC An address unique to T in the whole program, without RTTI: compares equal only for
		   the same T. */
		template <typename T>
		inline constexpr const void* type_key = &ca::impl::type_key_of<T>;

	} // namespace impl

} // namespace ca

#endif // ca_primitives_utility_type_key_hpp
