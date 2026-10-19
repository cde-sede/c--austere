// expect: exit 0
// mode: release
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "3-system/trace/of.hpp"
#include "4-lifecycle/context_manager.hpp"
#include "4-lifecycle/ctx.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "6-resources/fs/descriptor.hpp"

struct load_ctx : ca::context_manager<load_ctx> {
	ca::fs::descriptor file = ca::fs::descriptor::adopt(-1);
	ca::dynamic_array<ca::byte>* out = nullptr;

	auto enter(const char* const path, ca::dynamic_array<ca::byte>& dst) noexcept -> void {
		this->out = &dst;
		this->file = *ca::fs::descriptor::open(path);
		dst = *this->file.read_all();
	}

	auto leave(const ca::error* const error) noexcept -> void {
		if (this->file.is_open())
			static_cast<void>(this->file.cleanup());
		if (error != nullptr && this->out != nullptr)
			static_cast<void>(this->out->cleanup());
	}
};

static auto load(const char* const path, ca::dynamic_array<ca::byte>& out) noexcept -> ca::result<void> {
	return ca::ctx<load_ctx>::run(path, out);
}

static auto traced(void) noexcept -> ca::result<int> {
	return ca::trace::fail(ca::errc::invalid_argument, "bad input");
}

struct traced_ctx : ca::context_manager<traced_ctx> {
	int left = 0;
	bool failed = false;

	auto enter(int& reached) noexcept -> void {
		reached = *traced();
		reached = 1;
	}

	auto leave(const ca::error* const error) noexcept -> void {
		++this->left;
		this->failed = error != nullptr;
	}
};

struct checked_ctx : ca::context_manager<checked_ctx> {
	auto enter(const bool fail) noexcept -> ca::result<void> {
		if (fail)
			return ca::trace::fail(ca::errc::exists, "returned");
		return {};
	}

	auto leave(const ca::error*) noexcept -> void {
	}
};

auto main(void) -> int {
	ca::dynamic_array<ca::byte> contents{};
	auto loaded = load("/proc/self/comm", contents);
	if (!loaded || contents.size() == 0U)
		return 1;
	static_cast<void>(contents.cleanup());

	auto missing = load("/nonexistent/ca/context", contents);
	if (missing || missing.error() != ca::errc::no_entry || contents.size() != 0U)
		return 2;
	const ca::trace::record record = ca::trace::of(missing.error());
	if (record.frames().size() != 1U || record.frames()[0].message() != ca::sv::str{"value accessed while absent"})
		return 3;

	int reached = 0;
	traced_ctx tracing{};
	auto escaped = tracing.run(reached);
	if (escaped || escaped.error() != ca::errc::invalid_argument || reached != 0)
		return 4;
	if (tracing.left != 1 || !tracing.failed)
		return 8;
	const ca::trace::record chain = ca::trace::of(escaped.error());
	if (chain.frames().size() != 2U || chain.frames()[0].message() != ca::sv::str{"bad input"})
		return 5;

	auto returned = ca::ctx<checked_ctx>::run(true);
	if (returned || returned.error() != ca::errc::exists)
		return 6;
	auto passed = checked_ctx{}.run(false);
	if (!passed)
		return 7;
	return 0;
}
