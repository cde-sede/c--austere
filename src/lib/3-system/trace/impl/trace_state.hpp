#ifndef ca_system_trace_impl_trace_state_hpp
#define ca_system_trace_impl_trace_state_hpp

#include "1-primitives/debug/source_location.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/backtrace.hpp"
#include "3-system/trace/frame.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE STATE --------------------------------------------------------

	namespace trace {


		namespace impl {

			inline constexpr ca::usz CAPACITY = 32U;

			/* @DOC The thread's one trace: the error it follows (id, code), its first CAPACITY
			   frames, later ones counted in dropped, and the backtrace of the first trap it met
			   (empty when none). */
			struct trace_state final {

					using self = ca::trace::impl::trace_state;

					ca::u32 _id;
					int _code;
					ca::usz _size;
					ca::usz _dropped;
					ca::trace::frame _frames[CAPACITY];
					ca::trace::backtrace _backtrace;

			}; // struct trace_state

			extern constinit thread_local ca::trace::impl::trace_state current;

			/* @DOC Replaces the thread's trace with a new one for error's code, first frame at where;
			   returns the error numbered with it. */
			[[nodiscard]] auto start(ca::error error, ca::sv::str message, ca::source_location where) noexcept -> ca::error;

			auto push(ca::sv::str message, ca::source_location where) noexcept -> void;

		} // namespace impl

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_impl_trace_state_hpp
