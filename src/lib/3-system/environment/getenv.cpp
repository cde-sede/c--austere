#include "2-vocabulary/nullopt.hpp"
#include "3-system/environment/getenv.hpp"

#include <stdlib.h>


[[nodiscard]] auto ca::sys::getenv(const char* const name) noexcept -> ca::optional<ca::sv::str> {

	const char* const value = ::getenv(name);
	if (value == nullptr)
		return ca::nullopt;
	return ca::sv::str{value};
}
