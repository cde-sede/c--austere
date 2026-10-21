#ifndef ca_system_process_spawn_actions_hpp
#define ca_system_process_spawn_actions_hpp

#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/impl/access.hpp"
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
		   program (posix_spawn_file_actions_t). From make(); cleanup() frees them. Move-only: the
		   list holds no pointer to itself (glibc, musl, libca_runtime), so moving copies its
		   bytes and leaves the source empty. */
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

				/* @DOC Not live: what moving and cleanup() leave. make() builds a live one. */
				spawn_actions(void) noexcept
				: _storage{}, _live{false} {
				}

				spawn_actions(const self&) = delete;

				spawn_actions(self&& other) noexcept
				: _storage{}, _live{ca::exchange(other._live, false)} {
					ca::copy_bytes(_storage, other._storage, sizeof(_storage));
				}

				// Failsafe for a skipped cleanup(): frees the list, ignoring errors.
				// ~spawn_actions(void) noexcept;

				/* @DOC An empty list, live. */
				[[nodiscard]] static auto make(void) noexcept -> ca::result<self>;


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				/* @DOC Takes other's list. Traps when this one is live, its release being fallible:
				   cleanup() first. */
				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					if (_live) [[unlikely]]
						ca::impl::assigned_over_an_uncleaned_object();
					ca::copy_bytes(_storage, other._storage, sizeof(_storage));
					_live = ca::exchange(other._live, false);
					return *this;
				}


				// -- accessors ----------------------------------------------

				/* @DOC Whether make() built it and cleanup() has not run. */
				[[nodiscard]] constexpr auto is_live(void) const noexcept -> bool {
					return _live;
				}


				// -- methods ------------------------------------------------

				/* @DOC In the child, fd to becomes a copy of from, without close-on-exec. */
				[[nodiscard]] auto add_dup2(int from, int to) noexcept -> ca::result<void>;

				/* @DOC In the child, fd is closed. */
				[[nodiscard]] auto add_close(int fd) noexcept -> ca::result<void>;

				/* @DOC In the child, path is opened as fd (raw open flags and mode). path must outlive
				   the spawn. */
				[[nodiscard]] auto add_open(int fd, const char* path, int flags, ca::u32 mode) noexcept -> ca::result<void>;

				/* @DOC Frees the list: not live after, even on failure. One not live succeeds. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void>;

		}; // class spawn_actions

	} // namespace sys

} // namespace ca

#endif // ca_system_process_spawn_actions_hpp
