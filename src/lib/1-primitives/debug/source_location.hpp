#ifndef ca_primitives_debug_source_location_hpp
#define ca_primitives_debug_source_location_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SOURCE LOCATION ----------------------------------------------------

	/* @DOC A place in the source: file as given to the compiler, enclosing function name, line.
	   As a defaulted parameter, `ca::source_location where = ca::source_location::current()`
	   names the caller. Both strings are literals, static storage. */
	class source_location final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::source_location;


			// -- private members --------------------------------------------

			const char* _file;
			const char* _function;
			ca::u32 _line;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr source_location(void) noexcept
			: _file{""}, _function{""}, _line{0U} {
			}

			constexpr source_location(const char* const file, const char* const function, const ca::u32 line) noexcept
			: _file{file}, _function{function}, _line{line} {
			}

			/* @DOC Where it is written, or the caller's location when it is a default argument. */
			[[nodiscard]] static constexpr auto current(const char* const file = __builtin_FILE(),
			                                            const char* const function = __builtin_FUNCTION(),
			                                            const ca::u32 line = static_cast<ca::u32>(__builtin_LINE())) noexcept -> self {
				return self{file, function, line};
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto file(void) const noexcept -> const char* {
				return _file;
			}

			[[nodiscard]] constexpr auto function(void) const noexcept -> const char* {
				return _function;
			}

			[[nodiscard]] constexpr auto line(void) const noexcept -> ca::u32 {
				return _line;
			}

	}; // class source_location

} // namespace ca

#endif // ca_primitives_debug_source_location_hpp
