#ifndef ca_vocabulary_error_hpp
#define ca_vocabulary_error_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ERROR --------------------------------------------------------------

	/* @DOC Error code, 0 meaning success. Codes carry no meaning at this layer; errno values come
	   from the system layer. An error may also name the trace that follows it (ca::trace), 0 for
	   none; equality compares codes only. */
	class error final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::error;


			// -- private members --------------------------------------------

			int _code;
			ca::u32 _trace;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr error(void) noexcept
			: _code{0}, _trace{0U} {
			}

			constexpr explicit error(const int code) noexcept
			: _code{code}, _trace{0U} {
			}

			/* @DOC code, followed by the thread trace numbered trace. */
			constexpr error(const int code, const ca::u32 trace) noexcept
			: _code{code}, _trace{trace} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto ok(void) const noexcept -> bool {
				return _code == 0;
			}

			[[nodiscard]] constexpr auto code(void) const noexcept -> int {
				return _code;
			}

			/* @DOC The number of the trace that follows it, 0 for none. */
			[[nodiscard]] constexpr auto trace(void) const noexcept -> ca::u32 {
				return _trace;
			}

			[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
				return _code == other._code;
			}

	}; // class error

} // namespace ca

#endif // ca_vocabulary_error_hpp
