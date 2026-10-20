#ifndef ca_primitives_debug_frame_address_hpp
#define ca_primitives_debug_frame_address_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FRAME ADDRESS ------------------------------------------------------

	namespace impl {

		/* @DOC The frame record of the function this is inlined into: the caller's frame pointer,
		   then the return address. Forces that function to keep a frame pointer. */
		[[nodiscard, gnu::always_inline]] inline auto frame_address(void) noexcept -> const void* {
			return __builtin_frame_address(0);
		}

	} // namespace impl

} // namespace ca

#endif // ca_primitives_debug_frame_address_hpp
