#ifndef ca_system_io_pipe_ends_hpp
#define ca_system_io_pipe_ends_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- PIPE ENDS ------------------------------------------------------

		/* @DOC The two descriptors of a pipe; the caller closes both. */
		class pipe_ends final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::pipe_ends;


				// -- private members ----------------------------------------

				int _read;
				int _write;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr pipe_ends(const int read, const int write) noexcept
				: _read{read}, _write{write} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC The end to read from. */
				[[nodiscard]] constexpr auto read_end(void) const noexcept -> int {
					return _read;
				}

				/* @DOC The end to write to. */
				[[nodiscard]] constexpr auto write_end(void) const noexcept -> int {
					return _write;
				}

		}; // class pipe_ends

	} // namespace sys

} // namespace ca

#endif // ca_system_io_pipe_ends_hpp
