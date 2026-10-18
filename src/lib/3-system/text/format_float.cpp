#include "3-system/text/format_float.hpp"

#include <stdio.h>


auto ca::sys::format_float(const ca::span<char> buffer, const ca::f64 value, const char conversion, const ca::sint precision, const bool alternate) noexcept -> ca::usz {

	char* const out = buffer.data();
	const ca::usz capacity = buffer.size();
	if (capacity == 0U) [[unlikely]]
		return 0U;
	char spec[8]{};
	ca::usz at = 0U;
	spec[at++] = '%';
	if (alternate)
		spec[at++] = '#';
	if (precision >= 0) {
		spec[at++] = '.';
		spec[at++] = '*';
	}
	spec[at] = conversion;
	const int written = precision >= 0 ? ::snprintf(out, capacity, spec, precision, value) : ::snprintf(out, capacity, spec, value);
	if (written < 0) [[unlikely]] {
		out[0] = '\0';
		return 0U;
	}
	return static_cast<ca::usz>(written) < capacity ? static_cast<ca::usz>(written) : capacity - 1U;
}
