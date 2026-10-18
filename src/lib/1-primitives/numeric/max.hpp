#ifndef ca_primitives_numeric_max_hpp
#define ca_primitives_numeric_max_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAX ----------------------------------------------------------------

	/* @DOC The greater of a and b; a when equal. */
	template <typename T>
	[[nodiscard]] constexpr auto max(const T& a, const T& b) noexcept -> const T& {
		return a < b ? b : a;
	}

} // namespace ca

#endif // ca_primitives_numeric_max_hpp
