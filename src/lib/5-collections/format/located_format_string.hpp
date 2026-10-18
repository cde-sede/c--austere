#ifndef ca_collections_format_located_format_string_hpp
#define ca_collections_format_located_format_string_hpp

#include "1-primitives/debug/source_location.hpp"
#include "5-collections/format/format_string.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LOCATED FORMAT STRING ----------------------------------------------

	/* @DOC A format_string and where it is written: a parameter of this type takes a literal and
	   records the caller's location, which a defaulted parameter after a pack cannot. */
	template <typename... Args>
	class located_format_string final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::located_format_string<Args...>;


			// -- private members --------------------------------------------

			ca::format_string<Args...> _format;
			ca::source_location _where;


		public:

			// -- public lifecycle -------------------------------------------

			consteval located_format_string(const char* const text, const ca::source_location where = ca::source_location::current()) noexcept
			: _format{text}, _where{where} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto format(void) const noexcept -> const ca::format_string<Args...>& {
				return _format;
			}

			[[nodiscard]] constexpr auto where(void) const noexcept -> ca::source_location {
				return _where;
			}

	}; // class located_format_string

} // namespace ca

#endif // ca_collections_format_located_format_string_hpp
