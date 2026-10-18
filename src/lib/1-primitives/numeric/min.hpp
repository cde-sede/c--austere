#ifndef ca_primitives_numeric_min_hpp
#define ca_primitives_numeric_min_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MIN ----------------------------------------------------------------

	/* @DOC The lesser of a and b; a when equal. */
	template <typename T>
	[[nodiscard]] constexpr auto min(const T& a, const T& b) noexcept -> const T& {
		return b < a ? b : a;
	}

} // namespace ca

#endif // ca_primitives_numeric_min_hpp
