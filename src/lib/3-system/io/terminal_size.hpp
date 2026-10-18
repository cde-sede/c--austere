#ifndef ca_system_io_terminal_size_hpp
#define ca_system_io_terminal_size_hpp

#include "1-primitives/integers/u16.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TERMINAL SIZE --------------------------------------------------

		/* @DOC Size of a terminal window, in character cells. */
		class terminal_size final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::terminal_size;


				// -- private members ----------------------------------------

				ca::u16 _rows;
				ca::u16 _columns;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr terminal_size(const ca::u16 rows, const ca::u16 columns) noexcept
				: _rows{rows}, _columns{columns} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto rows(void) const noexcept -> ca::u16 {
					return _rows;
				}

				[[nodiscard]] constexpr auto columns(void) const noexcept -> ca::u16 {
					return _columns;
				}

		}; // class terminal_size

	} // namespace sys

} // namespace ca

#endif // ca_system_io_terminal_size_hpp
