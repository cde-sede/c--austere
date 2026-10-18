#include "3-system/text/format_float_shortest.hpp"

#include <stdio.h>
#include <stdlib.h>


[[nodiscard]] static auto clamp_written(const int written, char* const out, const ca::usz capacity) noexcept -> ca::usz {
	if (written < 0) [[unlikely]] {
		out[0] = '\0';
		return 0U;
	}
	return static_cast<ca::usz>(written) < capacity ? static_cast<ca::usz>(written) : capacity - 1U;
}

auto ca::sys::format_float_shortest(const ca::span<char> buffer, const ca::f64 value) noexcept -> ca::usz {

	char* const out = buffer.data();
	const ca::usz capacity = buffer.size();
	if (capacity == 0U) [[unlikely]]
		return 0U;
	if (value != value || value - value != 0.0)
		return clamp_written(::snprintf(out, capacity, "%g", value), out, capacity);
	int written = -1;
	for (int digits = 15; digits <= 17; ++digits) {
		written = ::snprintf(out, capacity, "%.*g", digits, value);
		if (written < 0 || static_cast<ca::usz>(written) >= capacity || ::strtod(out, nullptr) == value)
			break;
	}
	return clamp_written(written, out, capacity);
}

auto ca::sys::format_float_shortest(const ca::span<char> buffer, const ca::f32 value) noexcept -> ca::usz {

	char* const out = buffer.data();
	const ca::usz capacity = buffer.size();
	if (capacity == 0U) [[unlikely]]
		return 0U;
	if (value != value || value - value != 0.0F)
		return clamp_written(::snprintf(out, capacity, "%g", static_cast<double>(value)), out, capacity);
	int written = -1;
	for (int digits = 6; digits <= 9; ++digits) {
		written = ::snprintf(out, capacity, "%.*g", digits, static_cast<double>(value));
		if (written < 0 || static_cast<ca::usz>(written) >= capacity || ::strtof(out, nullptr) == value)
			break;
	}
	return clamp_written(written, out, capacity);
}
