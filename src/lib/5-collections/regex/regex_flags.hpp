#ifndef ca_collections_regex_regex_flags_hpp
#define ca_collections_regex_regex_flags_hpp

#include "1-primitives/integers/u8.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX FLAGS --------------------------------------------------------

	/* @DOC The flags of Python's re, combined with `|`: ignore_case (re.I: Unicode caseless
	   classes as re takes them), multiline (re.M: `^` and `$` at every line), dot_all (re.S:
	   `.` takes '\n' too), verbose (re.X: white space and `#` comments outside classes
	   ignored), ascii (re.A: `\w \d \s \b` and ignore_case on ASCII only). The pattern sets
	   them inline too: `(?imsxa)` at its start, `(?i:...)` and `(?-i:...)` for a group. */
	enum class regex_flags : ca::u8 {
		none        = 0U,
		ignore_case = 1U << 0U,
		multiline   = 1U << 1U,
		dot_all     = 1U << 2U,
		verbose     = 1U << 3U,
		ascii       = 1U << 4U,
	};

	[[nodiscard]] constexpr auto operator|(const ca::regex_flags left, const ca::regex_flags right) noexcept -> ca::regex_flags {
		return static_cast<ca::regex_flags>(static_cast<ca::u8>(left) | static_cast<ca::u8>(right));
	}

	[[nodiscard]] constexpr auto operator&(const ca::regex_flags left, const ca::regex_flags right) noexcept -> ca::regex_flags {
		return static_cast<ca::regex_flags>(static_cast<ca::u8>(left) & static_cast<ca::u8>(right));
	}

	[[nodiscard]] constexpr auto operator~(const ca::regex_flags flags) noexcept -> ca::regex_flags {
		return static_cast<ca::regex_flags>(~static_cast<ca::u8>(flags) & 0x1FU);
	}

} // namespace ca

#endif // ca_collections_regex_regex_flags_hpp
