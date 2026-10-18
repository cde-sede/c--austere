#ifndef ca_primitives_numeric_align_up_hpp
#define ca_primitives_numeric_align_up_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ALIGN UP -----------------------------------------------------------

	/* @DOC Rounds value up to a multiple of alignment, a power of two. True on overflow, value then
	   unchanged. */
	[[nodiscard]] constexpr auto align_up(ca::usz& value, const ca::usz alignment) noexcept -> bool {

		const ca::usz mask = alignment - 1U;
		if (value > ca::limits<ca::usz>::max() - mask) [[unlikely]]
			return true;
		value = (value + mask) & ~mask;
		return false;
	}

} // namespace ca

#endif // ca_primitives_numeric_align_up_hpp
