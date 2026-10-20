#ifndef ca_vocabulary_ask_hpp
#define ca_vocabulary_ask_hpp

#include "1-primitives/debug/frame_address.hpp"
#include "1-primitives/debug/recovery.hpp"
#include "1-primitives/debug/source_location.hpp"
#include "2-vocabulary/error.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ASK ----------------------------------------------------------------

	namespace impl {

		[[gnu::cold, gnu::noinline]] inline auto ask(const char* const reason, const ca::error error, const ca::source_location where) noexcept -> bool {

			ca::impl::trap_info info{};
			   info._reason = reason;
			    info._frame = ca::impl::frame_address();
			    info._where = where;
			     info._code = error.code();
			    info._trace = error.trace();
			info._resumable = true;
			return ca::impl::raise(info);
		}

	} // namespace impl


	/* @DOC A trap the calling code can come back from, for a point written to carry on: the
	   contexts decide as for ca::trap. true when one resumed it (it fixed the cause: check again),
	   false when none took it (fail the usual way); when one takes it, it unwinds there like a
	   trap. Handlers see error, if any, as its error. reason is a literal. */
	[[nodiscard]] inline auto ask(const char* const reason, const ca::error error = {}, const ca::source_location where = ca::source_location::current()) noexcept -> bool {

		if (ca::impl::current_recovery == nullptr)
			return false;
		return ca::impl::ask(reason, error, where);
	}

} // namespace ca

#endif // ca_vocabulary_ask_hpp
