#ifndef ca_system_process_spawn_options_hpp
#define ca_system_process_spawn_options_hpp

#include "1-primitives/integers/i32.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SPAWN OPTIONS --------------------------------------------------

		/* @DOC What posix_spawn sets up in the child besides its descriptors (posix_spawnattr_t,
		   held by value: nothing to free). Default: nothing. */
		class spawn_options final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::spawn_options;


				// -- private members ----------------------------------------

				ca::i32 _process_group;
				bool _sets_process_group;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr spawn_options(void) noexcept
				: _process_group{0}, _sets_process_group{false} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC The group the child joins, when set: 0 for a new one it leads. */
				[[nodiscard]] constexpr auto process_group(void) const noexcept -> ca::optional<ca::i32> {

					if (!_sets_process_group)
						return ca::nullopt;
					return _process_group;
				}


				// -- modifiers ----------------------------------------------

				/* @DOC The child joins process group group before it runs the program, as
				   setpgid(0, group) would: 0 makes a new group it leads, the way a shell starts
				   a job's first process. */
				constexpr auto set_process_group(const ca::i32 group) noexcept -> self& {

					     _process_group = group;
					_sets_process_group = true;
					return *this;
				}

		}; // class spawn_options

	} // namespace sys

} // namespace ca

#endif // ca_system_process_spawn_options_hpp
