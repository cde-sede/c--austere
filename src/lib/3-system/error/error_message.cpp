#include "1-primitives/config/platform.hpp"
#include "3-system/error/error_message.hpp"

#include <string.h>


[[nodiscard]] auto ca::sys::error_message(const ca::error error) noexcept -> ca::sv::str {

#if ca_platform(linux)
	const char* const text = ::strerrordesc_np(error.code());
#else
	const char* const text = error.code() > 0 && error.code() < ::sys_nerr ? ::sys_errlist[error.code()] : nullptr;
#endif
	if (text == nullptr)
		return ca::sv::str{"Unknown error"};
	return ca::sv::str{text};
}
