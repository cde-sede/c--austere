#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/process/execvp.hpp"

#include <unistd.h>


[[nodiscard]] auto ca::sys::execvp(const char* const file, char* const* const argv) noexcept -> ca::result<void> {

	::execvp(file, argv);
	return ca::failure{ca::sys::impl::libc_error()};
}
