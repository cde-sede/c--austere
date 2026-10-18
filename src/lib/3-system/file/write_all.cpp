#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/file/write.hpp"
#include "3-system/file/write_all.hpp"


[[nodiscard]] auto ca::sys::write_all(const int fd, const ca::span<const ca::byte> bytes) noexcept -> ca::result<void> {

	ca::span<const ca::byte> left = bytes;
	while (!left.empty()) {
		auto written = ca::sys::write(fd, left);
		if (!written) [[unlikely]]
			return written.propagate();
		left = left.without_prefix(*written);
	}
	return ca::result<void>{};
}
