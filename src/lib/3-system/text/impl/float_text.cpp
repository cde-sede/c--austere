#include "3-system/text/impl/float_text.hpp"


static auto digit_at(const ca::sys::impl::float_digits& digits, const ca::i64 index) noexcept -> char {
	return index >= 0 && static_cast<ca::usz>(index) < digits._count ? digits._digits[index] : '0';
}

auto ca::sys::impl::put(ca::sys::impl::float_sink& sink, const char c) noexcept -> void {

	if (sink._length + 1U < sink._capacity)
		sink._out[sink._length] = c;
	++sink._length;
}

[[nodiscard]] auto ca::sys::impl::finish(ca::sys::impl::float_sink& sink) noexcept -> ca::usz {

	const ca::usz written = sink._length < sink._capacity ? sink._length : sink._capacity - 1U;
	sink._out[written] = '\0';
	return written;
}

auto ca::sys::impl::put_special(ca::sys::impl::float_sink& sink, const bool negative, const bool infinite, const bool upper) noexcept -> void {

	if (negative)
		put(sink, '-');
	const char* const text = infinite ? (upper ? "INF" : "inf") : (upper ? "NAN" : "nan");
	for (const char* at = text; *at != '\0'; ++at)
		put(sink, *at);
}

auto ca::sys::impl::put_fixed(ca::sys::impl::float_sink& sink, const ca::sys::impl::float_digits& digits, const ca::i64 fraction, const bool always_point) noexcept -> void {

	if (digits._point <= 0) {
		put(sink, '0');
	} else {
		for (ca::i64 i = 0; i < digits._point; ++i)
			put(sink, digit_at(digits, i));
	}
	if (fraction > 0 || always_point)
		put(sink, '.');
	for (ca::i64 j = 0; j < fraction; ++j)
		put(sink, digit_at(digits, digits._point + j));
}

auto ca::sys::impl::put_scientific(ca::sys::impl::float_sink& sink, const ca::sys::impl::float_digits& digits, const ca::i64 fraction, const bool always_point, const char exponent_letter) noexcept -> void {

	put(sink, digit_at(digits, 0));
	if (fraction > 0 || always_point)
		put(sink, '.');
	for (ca::i64 i = 1; i <= fraction; ++i)
		put(sink, digit_at(digits, i));
	put(sink, exponent_letter);
	const ca::i32 exponent = digits._count == 0U ? 0 : digits._point - 1;
	put(sink, exponent < 0 ? '-' : '+');
	const ca::i32 magnitude = exponent < 0 ? -exponent : exponent;
	char text[12];
	ca::usz length = 0U;
	for (ca::i32 rest = magnitude; rest != 0 || length < 2U; rest /= 10)
		text[length++] = static_cast<char>('0' + rest % 10);
	while (length != 0U)
		put(sink, text[--length]);
}

[[nodiscard]] auto ca::sys::impl::trimmed(const ca::sys::impl::float_digits& digits, const ca::i64 first, ca::i64 fraction) noexcept -> ca::i64 {

	const ca::i64 meaningful = static_cast<ca::i64>(digits._count) - first;
	if (fraction > meaningful)
		fraction = meaningful > 0 ? meaningful : 0;
	while (fraction > 0 && digit_at(digits, first + fraction - 1) == '0')
		--fraction;
	return fraction;
}
