#ifndef ca_collections_regex_impl_regex_names_hpp
#define ca_collections_regex_impl_regex_names_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/regex/impl/regex_program.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX NAMES --------------------------------------------------------

	namespace impl {

		/* @DOC A pattern's group names, borrowed: the entries and the text they point into. */
		struct regex_names final {

				using self = ca::impl::regex_names;

				ca::borrowed_slice<const ca::impl::regex_name> _entries;
				ca::sv::str                                    _text;

				/* @DOC The group named name, or nothing. */
				[[nodiscard]] constexpr auto find(const ca::sv::str name) const noexcept -> ca::optional<ca::u32> {

					for (const ca::impl::regex_name& entry : _entries) {
						if (ca::sv::str{_text.data() + entry._name_at, entry._name_size} == name)
							return entry._group;
					}
					return ca::nullopt;
				}

				/* @DOC The name of group, or nothing. */
				[[nodiscard]] constexpr auto name_of(const ca::u32 group) const noexcept -> ca::optional<ca::sv::str> {

					for (const ca::impl::regex_name& entry : _entries) {
						if (entry._group == group)
							return ca::sv::str{_text.data() + entry._name_at, entry._name_size};
					}
					return ca::nullopt;
				}

		}; // struct regex_names

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_names_hpp
