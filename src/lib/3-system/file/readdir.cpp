#include "2-vocabulary/failure.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/file/readdir.hpp"

#include <dirent.h>
#include <errno.h>


[[nodiscard]] auto ca::sys::readdir(ca::sys::directory_stream* const stream) noexcept -> ca::result<ca::sv::str> {
	for (;;) {
		errno = 0;
		const dirent* const entry = ::readdir(reinterpret_cast<DIR*>(stream));
		if (entry == nullptr) {
			if (errno != 0) [[unlikely]]
				return ca::failure{ca::sys::impl::libc_error()};
			return ca::sv::str{};
		}
		const ca::sv::str name{entry->d_name};
		if (name != ca::sv::str{"."} && name != ca::sv::str{".."})
			return name;
	}
}
