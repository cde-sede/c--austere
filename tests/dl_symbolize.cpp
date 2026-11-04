// expect: exit 0
// shared: symbols dl/symbols.cpp
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/backtrace.hpp"
#include "3-system/trace/location.hpp"
#include "5-collections/format/format.hpp"
#include "6-resources/dl/library.hpp"
#include "6-resources/trace/symbolizer.hpp"

static ca::trace::backtrace captured{};

[[gnu::noinline]] static auto capture(void) -> void {
	captured = ca::trace::backtrace::capture();
}

static const ca::dl::host_symbol HOST[] = {{"host_capture", reinterpret_cast<void*>(&capture)}};

/* The outermost function found at address contains name (and, built with -g, comes from
   file). */
static auto names(const ca::trace::symbolizer& symbols, const void* const address, const ca::sv::str name, const ca::sv::str file = "symbols.cpp") noexcept -> bool {

	ca::trace::location found[ca::trace::location::SCOPES];
	const ca::usz count = symbols.lookup(reinterpret_cast<ca::usz>(address) + 1U, found);
	if (count == 0U || !found[count - 1U].function().contains(name))
		return false;
#ifndef NDEBUG
	if (!found[count - 1U].file().ends_with(file) || found[count - 1U].line() == 0U)
		return false;
#else
	static_cast<void>(file);
#endif
	return true;
}

auto main(void) -> int {
	auto before = ca::trace::symbolizer::make();
	if (!before)
		return 1;
	auto plugin = ca::dl::library::make(CA_TEST_LIBRARIES "/libsymbols.so", HOST);
	if (!plugin)
		return 2;
	const auto entry = plugin->find("plugin_entry");
	const auto deep = plugin->find("_ZN6plugin4deepEi");
	const auto call_back = plugin->find_as<int(void)>("plugin_call_back");
	if (!entry || !deep || !call_back)
		return 3;
	ca::trace::location found[ca::trace::location::SCOPES];
	if (before->lookup(reinterpret_cast<ca::usz>(*entry) + 1U, found) != 0U)
		return 4;

	auto symbols = ca::trace::symbolizer::make();
	if (!symbols)
		return 5;
	if (!names(*symbols, *entry, "plugin_entry") || !names(*symbols, *deep, "deep"))
		return 6;
	if (!names(*symbols, reinterpret_cast<const void*>(&capture), "capture", "dl_symbolize.cpp"))
		return 7;

#ifndef NDEBUG
	if ((*call_back)() != 1 || captured.empty())
		return 8;
	auto printed = ca::format("{}", symbols->of(captured.addresses()));
	const bool through = printed && printed->view().contains("plugin_call_back") && printed->view().contains("main");
	if (printed)
		static_cast<void>(printed->cleanup());
	if (!through)
		return 9;
#endif

	if (!plugin->cleanup() || !symbols->cleanup() || !before->cleanup())
		return 10;
	return 0;
}
