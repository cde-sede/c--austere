#ifndef ca_system_io_poll_entry_hpp
#define ca_system_io_poll_entry_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- POLL ENTRY -----------------------------------------------------

		namespace impl {

			struct poll_layout;

		} // namespace impl


		/* @DOC One descriptor to watch with poll: the events requested (sys::poll_events), and those
		   reported. Same layout as struct pollfd, so arrays pass to the kernel uncopied. */
		class poll_entry final {


			private:

				// -- friends ------------------------------------------------

				friend struct ca::sys::impl::poll_layout;


				// -- private types ------------------------------------------

				using self = ca::sys::poll_entry;


				// -- private members ----------------------------------------

				int _fd;
				short _events;
				short _reported;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr poll_entry(const int fd, const short events) noexcept
				: _fd{fd}, _events{events}, _reported{0} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto fd(void) const noexcept -> int {
					return _fd;
				}

				/* @DOC Events requested. */
				[[nodiscard]] constexpr auto events(void) const noexcept -> short {
					return _events;
				}

				/* @DOC Events the last poll reported. */
				[[nodiscard]] constexpr auto reported(void) const noexcept -> short {
					return _reported;
				}

				/* @DOC Whether the last poll reported any of events. */
				[[nodiscard]] constexpr auto has(const short events) const noexcept -> bool {
					return (_reported & events) != 0;
				}

		}; // class poll_entry

	} // namespace sys

} // namespace ca

#endif // ca_system_io_poll_entry_hpp
