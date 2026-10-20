// expect: exit 0
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/backtrace.hpp"
#include "3-system/trace/location.hpp"
#include "6-resources/trace/symbolizer.hpp"

struct text final {
	char data[4096];
	ca::usz size;

	auto put(const char c) noexcept -> void {
		if (this->size < sizeof(this->data))
			this->data[this->size++] = c;
	}

	auto put(const ca::sv::str s) noexcept -> void {
		for (const char c : s)
			this->put(c);
	}

	auto view(void) const noexcept -> ca::sv::str {
		return ca::sv::str{this->data, this->size};
	}
};

[[gnu::noinline]] static auto where(void) noexcept -> ca::trace::backtrace {
	return ca::trace::backtrace::capture();
}

auto main(void) -> int {
	ca::trace::location found[ca::trace::location::SCOPES];
	ca::trace::symbolizer empty{};
	if (empty.lookup(0x1000U, found) != 0U || !empty.cleanup())
		return 1;

	const ca::trace::backtrace here = where();
	if (here.empty())
		return 2;
	auto symbols = ca::trace::symbolizer::make();
	if (!symbols)
		return 3;
	const ca::usz count = symbols->lookup(here.addresses()[0], found);
	if (count == 0U)
		return 4;
	const ca::trace::location& outermost = found[count - 1U];
	if (!outermost.function().contains(ca::sv::str{"where"}) || !outermost.offset().holds() || outermost.inlined())
		return 5;
	if (symbols->lookup(0x10U, found) != 0U || symbols->lookup(here.addresses()[0], ca::span<ca::trace::location>{}) != 0U)
		return 6;

	text printed{};
	symbols->of(here.addresses()).display(printed);
	if (!printed.view().starts_with(ca::sv::str{"    #0 "}) || !printed.view().contains(ca::sv::str{"where"}))
		return 7;

	ca::trace::symbolizer moved{static_cast<ca::trace::symbolizer&&>(*symbols)};
	if (symbols->lookup(here.addresses()[0], found) != 0U || moved.lookup(here.addresses()[0], found) == 0U)
		return 8;
	return moved.cleanup() && symbols->cleanup() ? 0 : 9;
}
