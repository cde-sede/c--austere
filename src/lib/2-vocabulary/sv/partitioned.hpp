#ifndef ca_vocabulary_sv_partitioned_hpp
#define ca_vocabulary_sv_partitioned_hpp

#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/sv/impl/forward.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SV -----------------------------------------------------------------

	namespace sv {

		// -- PARTITIONED ----------------------------------------------------

		/* @DOC What partition and partition_last return (str.partition): the text before the
		   separator, the separator, the text after. Not found: partition gives (text, "", ""),
		   partition_last ("", "", text), as Python does. */
		template <ca::is_character C>
		class partitioned final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sv::partitioned<C>;


				// -- private members ----------------------------------------

				ca::sv::basic<C> _before;
				ca::sv::basic<C> _separator;
				ca::sv::basic<C> _after;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr partitioned(const ca::sv::basic<C> before, const ca::sv::basic<C> separator, const ca::sv::basic<C> after) noexcept
				: _before{before}, _separator{separator}, _after{after} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto before(void) const noexcept -> ca::sv::basic<C> {
					return _before;
				}

				/* @DOC The separator found, empty when there was none. */
				[[nodiscard]] constexpr auto separator(void) const noexcept -> ca::sv::basic<C> {
					return _separator;
				}

				[[nodiscard]] constexpr auto after(void) const noexcept -> ca::sv::basic<C> {
					return _after;
				}

				[[nodiscard]] constexpr auto found(void) const noexcept -> bool {
					return !_separator.empty();
				}

		}; // class partitioned

	} // namespace sv

} // namespace ca

#endif // ca_vocabulary_sv_partitioned_hpp
