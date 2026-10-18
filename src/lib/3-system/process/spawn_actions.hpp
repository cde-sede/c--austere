#ifndef ca_system_process_spawn_actions_hpp
#define ca_system_process_spawn_actions_hpp

#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SPAWN ACTIONS --------------------------------------------------

		namespace impl {

			struct spawn_actions_layout;

		#if ca_platform(linux)
			inline constexpr ca::usz spawn_actions_size = 2U * sizeof(int) + sizeof(void*) + 16U * sizeof(int);
		#else
			inline constexpr ca::usz spawn_actions_size = sizeof(void*);
		#endif

		} // namespace impl


		/* @DOC Descriptor changes posix_spawn applies in the child, in order, before it runs the
		   program (posix_spawn_file_actions_t). init before use; destroy after; the destructor only
		   destroys as a failsafe. Neither copyable nor movable. */
		class spawn_actions final {


			private:

				// -- friends ------------------------------------------------

				friend struct ca::sys::impl::spawn_actions_layout;


				// -- private types ------------------------------------------

				using self = ca::sys::spawn_actions;


				// -- private members ----------------------------------------

				alignas(void*) ca::byte _storage[ca::sys::impl::spawn_actions_size];
				bool _live;


			public:

				// -- public lifecycle ---------------------------------------

				spawn_actions(void) noexcept
				: _storage{}, _live{false} {
				}

				spawn_actions(const self&) = delete;

				~spawn_actions(void) noexcept;


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;


				// -- accessors ----------------------------------------------

				/* @DOC Whether init succeeded and destroy has not run. */
				[[nodiscard]] constexpr auto is_live(void) const noexcept -> bool {
					return _live;
				}


				// -- methods ------------------------------------------------

				/* @DOC Starts an empty list. errc::invalid_argument when already live. */
				[[nodiscard]] auto init(void) noexcept -> ca::result<void>;

				/* @DOC In the child, fd to becomes a copy of from, without close-on-exec. */
				[[nodiscard]] auto add_dup2(int from, int to) noexcept -> ca::result<void>;

				/* @DOC In the child, fd is closed. */
				[[nodiscard]] auto add_close(int fd) noexcept -> ca::result<void>;

				/* @DOC In the child, path is opened as fd (raw open flags and mode). path must outlive
				   the spawn. */
				[[nodiscard]] auto add_open(int fd, const char* path, int flags, ca::u32 mode) noexcept -> ca::result<void>;

				/* @DOC Frees the list. errc::invalid_argument when not live. */
				[[nodiscard]] auto destroy(void) noexcept -> ca::result<void>;

		}; // class spawn_actions

	} // namespace sys

} // namespace ca

#endif // ca_system_process_spawn_actions_hpp
