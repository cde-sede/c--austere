#ifndef ca_primitives_numeric_sub_overflow_hpp
#define ca_primitives_numeric_sub_overflow_hpp

#include "1-primitives/traits/is_integral.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SUB OVERFLOW -------------------------------------------------------

	/* @DOC out = a - b - cs...; true on overflow (out then holds the wrapped value). */
	template <typename R, typename A, typename B, typename... Cs>
	requires (ca::is_integral<R> && ca::is_integral<A> && ca::is_integral<B> && (ca::is_integral<Cs> && ...))
	[[nodiscard]] constexpr auto sub_overflow(R& out, const A a, const B b, const Cs... cs) noexcept -> bool {
		return __builtin_sub_overflow(a, b, &out) || (... || __builtin_sub_overflow(out, cs, &out));
	}

} // namespace ca

#endif // ca_primitives_numeric_sub_overflow_hpp
