#ifndef ca_primitives_utility_addressof_hpp
#define ca_primitives_utility_addressof_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ADDRESSOF ----------------------------------------------------------

	/* @DOC Address of value, ignoring any overloaded operator&. Rvalues are rejected. */
	template <typename T>
	[[nodiscard]] constexpr auto addressof(T& value) noexcept -> T* {
		return __builtin_addressof(value);
	}

	template <typename T>
	auto addressof(const T&&) noexcept -> const T* = delete;

} // namespace ca

#endif // ca_primitives_utility_addressof_hpp
