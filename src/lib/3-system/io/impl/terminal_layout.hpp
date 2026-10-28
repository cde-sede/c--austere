#ifndef ca_system_io_impl_terminal_layout_hpp
#define ca_system_io_impl_terminal_layout_hpp

#include "3-system/io/terminal_settings.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TERMINAL LAYOUT ------------------------------------------------

		namespace impl {

			/* @DOC Copies between terminal_settings and struct termios, where the layouts differ. */
			struct terminal_layout final {

					using self = ca::sys::impl::terminal_layout;

					/* @DOC Fills struct termios native from settings. */
					static auto to_native(const ca::sys::terminal_settings& settings, void* native) noexcept -> void;

					/* @DOC The settings in struct termios native. */
					[[nodiscard]] static auto from_native(const void* native) noexcept -> ca::sys::terminal_settings;

			}; // struct terminal_layout

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_io_impl_terminal_layout_hpp
