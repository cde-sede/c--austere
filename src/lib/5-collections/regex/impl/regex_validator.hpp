#ifndef ca_collections_regex_impl_regex_validator_hpp
#define ca_collections_regex_impl_regex_validator_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/regex/impl/regex_look.hpp"
#include "5-collections/regex/regex_flags.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX VALIDATOR ----------------------------------------------------

	namespace impl {

		/* @DOC The parser's sink that builds nothing: a syntax check, constexpr. Group names are
		   checked for duplicates among the first NAMES; past them, only at runtime. */
		class regex_validator final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::regex_validator;


			public:

				// -- public types -------------------------------------------

				static constexpr ca::usz NAMES = 32U;


			private:

				// -- private members ----------------------------------------

				ca::sv::str _names[NAMES];
				ca::usz     _count;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr regex_validator(void) noexcept
				: _names{}, _count{0U} {
				}


				// -- methods ------------------------------------------------

				[[nodiscard]] constexpr auto ok(void) const noexcept -> bool {
					return true;
				}

				[[nodiscard]] constexpr auto empty(void) noexcept -> ca::u32 {
					return 0U;
				}

				[[nodiscard]] constexpr auto literal(const char32_t, const ca::regex_flags) noexcept -> ca::u32 {
					return 0U;
				}

				[[nodiscard]] constexpr auto any(const ca::regex_flags) noexcept -> ca::u32 {
					return 0U;
				}

				[[nodiscard]] constexpr auto look(const ca::impl::regex_look) noexcept -> ca::u32 {
					return 0U;
				}

				constexpr auto set_begin(void) noexcept -> void {
				}

				constexpr auto set_range(const char32_t, const char32_t) noexcept -> void {
				}

				constexpr auto set_class(const ca::impl::regex_class, const bool, const ca::regex_flags) noexcept -> void {
				}

				[[nodiscard]] constexpr auto set_end(const bool, const ca::regex_flags) noexcept -> ca::u32 {
					return 0U;
				}

				[[nodiscard]] constexpr auto repeat(const ca::u32, const ca::u32, const ca::u32, const bool) noexcept -> ca::u32 {
					return 0U;
				}

				[[nodiscard]] constexpr auto group(const ca::u32, const ca::u32) noexcept -> ca::u32 {
					return 0U;
				}

				[[nodiscard]] constexpr auto name(const ca::sv::str name, const ca::u32) noexcept -> bool {

					for (ca::usz i = 0U; i < _count; ++i) {
						if (_names[i] == name)
							return false;
					}
					if (_count < NAMES)
						_names[_count++] = name;
					return true;
				}

				[[nodiscard]] constexpr auto concat(const ca::u32) noexcept -> ca::u32 {
					return 0U;
				}

				[[nodiscard]] constexpr auto alternate(const ca::u32) noexcept -> ca::u32 {
					return 0U;
				}

				constexpr auto link(const ca::u32, const ca::u32) noexcept -> void {
				}

				constexpr auto root(const ca::u32) noexcept -> void {
				}

		}; // class regex_validator

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_validator_hpp
