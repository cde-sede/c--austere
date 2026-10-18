#include "3-system/trace/fail.hpp"
#include "3-system/trace/impl/trace_state.hpp"


[[nodiscard, gnu::cold]] auto ca::trace::fail(const ca::error error, const ca::sv::str message, const ca::source_location where) noexcept -> ca::failure<ca::error> {
	return ca::failure{ca::trace::impl::start(error, message, where)};
}
