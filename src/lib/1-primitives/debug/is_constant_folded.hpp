#ifndef ca_primitives_debug_is_constant_folded_hpp
#define ca_primitives_debug_is_constant_folded_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CONSTANT FOLDED -------------------------------------------------

	/* @DOC True when the optimizer proves value is a constant at this call site, after inlining
	   (__builtin_constant_p). Always false at -O0. Forced inline so the answer belongs to the
	   caller's context. */
	template <typename T>
	[[gnu::always_inline]] constexpr auto is_constant_folded(const T value) noexcept -> bool {
		return __builtin_constant_p(value);
	}

} // namespace ca

#endif // ca_primitives_debug_is_constant_folded_hpp
