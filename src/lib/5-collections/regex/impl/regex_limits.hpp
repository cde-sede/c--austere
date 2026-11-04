#ifndef ca_collections_regex_impl_regex_limits_hpp
#define ca_collections_regex_impl_regex_limits_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX LIMITS -------------------------------------------------------

	/* @DOC Bounds of a pattern: group nesting, capture groups, a counted repeat's numbers and
	   the compiled program (instructions plus transitions), which bounds the memory a regex
	   takes and the time per text byte. */
	namespace impl {

		inline constexpr ca::usz regex_max_depth = 128U;

		inline constexpr ca::u32 regex_max_groups = 0xFFFFU;

		inline constexpr ca::u32 regex_max_repeat = 100000U;

		inline constexpr ca::u32 regex_max_program = 1U << 18U;

		inline constexpr ca::u32 regex_infinite = 0xFFFFFFFFU;

		inline constexpr ca::u32 regex_none = 0xFFFFFFFFU;

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_limits_hpp
