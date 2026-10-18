#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/file/write.hpp"
#include "6-resources/io/writer.hpp"


[[nodiscard]] auto ca::io::writer::_emit(const int fd, const char*& from, const char* const to) noexcept -> ca::error {

	while (from != to) {
		auto written = ca::sys::write(fd, ca::span{from, static_cast<ca::usz>(to - from)});
		if (!written) [[unlikely]]
			return written.error();
		from += *written;
	}
	return ca::error{};
}
