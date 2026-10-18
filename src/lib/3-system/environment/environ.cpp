#include "1-primitives/config/platform.hpp"
#include "3-system/environment/environ.hpp"


#if ca_platform(linux)
#	include <unistd.h>
#else
#	include <crt_externs.h>
#endif


[[nodiscard]] auto ca::sys::environ(void) noexcept -> char* const* {

#if ca_platform(linux)
	return ::environ;
#else
	return *::_NSGetEnviron();
#endif
}
