#ifndef ca_system_machine_system_name_hpp
#define ca_system_machine_system_name_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SYSTEM NAME ----------------------------------------------------

		/* @DOC What uname reports: the kernel, its release and version, the host and the hardware. Each
		   field keeps at most 255 chars. */
		class system_name final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::system_name;


				// -- private members ----------------------------------------

				char _text[5][256];
				ca::usz _sizes[5];


			public:

				// -- public lifecycle ---------------------------------------

				constexpr system_name(const ca::sv::str system, const ca::sv::str node, const ca::sv::str release, const ca::sv::str version, const ca::sv::str machine) noexcept
				: _text{}, _sizes{} {

					const ca::sv::str fields[5]{system, node, release, version, machine};
					for (ca::usz i = 0U; i < 5U; ++i) {
						_sizes[i] = fields[i].size() < 255U ? fields[i].size() : 255U;
						for (ca::usz j = 0U; j < _sizes[i]; ++j)
							_text[i][j] = fields[i][j];
					}
				}


				// -- accessors ----------------------------------------------

				/* @DOC Kernel name: "Linux", "Darwin". */
				[[nodiscard]] constexpr auto system(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_text[0], _sizes[0]};
				}

				/* @DOC Host name on the network. */
				[[nodiscard]] constexpr auto node(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_text[1], _sizes[1]};
				}

				/* @DOC Kernel release, as "6.8.0". */
				[[nodiscard]] constexpr auto release(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_text[2], _sizes[2]};
				}

				/* @DOC Kernel build version. */
				[[nodiscard]] constexpr auto version(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_text[3], _sizes[3]};
				}

				/* @DOC Hardware: "x86_64", "arm64". */
				[[nodiscard]] constexpr auto machine(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_text[4], _sizes[4]};
				}

		}; // class system_name

	} // namespace sys

} // namespace ca

#endif // ca_system_machine_system_name_hpp
