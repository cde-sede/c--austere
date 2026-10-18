#ifndef ca_system_io_isatty_hpp
#define ca_system_io_isatty_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ISATTY ---------------------------------------------------------

		/* @DOC Whether fd is a terminal. Never fails: false for anything else, closed descriptors included. */
		[[nodiscard]] auto isatty(int fd) noexcept -> bool;

	} // namespace sys

} // namespace ca

#endif // ca_system_io_isatty_hpp
