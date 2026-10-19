#include "1-primitives/integers/byte.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/impl/temporary_name.hpp"
#include "3-system/random/getentropy.hpp"


static constexpr char DIGITS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

[[nodiscard]] auto ca::sys::impl::temporary_suffix(char* const pattern) noexcept -> ca::result<char*> {

	const ca::usz length = ca::string_length(pattern);
	bool valid = length >= 6U;
	for (ca::usz i = 0U; valid && i < 6U; ++i)
		valid = pattern[length - 1U - i] == 'X';
	if (!valid) [[unlikely]] {
		ca::sys::set_last_error(ca::errc::invalid_argument);
		return ca::failure{ca::errc::invalid_argument};
	}
	return pattern + length - 6U;
}

[[nodiscard]] auto ca::sys::impl::randomize_suffix(char* const suffix) noexcept -> ca::result<void> {

	ca::byte noise[6];
	const auto filled = ca::sys::getentropy(ca::span<ca::byte>{noise, sizeof(noise)});
	if (!filled) [[unlikely]]
		return filled.propagate();
	for (ca::usz i = 0U; i < 6U; ++i)
		suffix[i] = DIGITS[noise[i] % (sizeof(DIGITS) - 1U)];
	return ca::result<void>{};
}
