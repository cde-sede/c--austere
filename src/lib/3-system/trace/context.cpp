#include "3-system/trace/context.hpp"
#include "3-system/trace/impl/trace_state.hpp"


[[nodiscard, gnu::cold]] auto ca::trace::context(const ca::error error, const ca::sv::str message, const ca::source_location where) noexcept -> ca::failure<ca::error> {

	if (error.trace() == 0U || error.trace() != ca::trace::impl::current._id) [[unlikely]]
		return ca::failure{ca::trace::impl::start(error, message, where)};
	ca::trace::impl::push(message, where);
	return ca::failure{error};
}
