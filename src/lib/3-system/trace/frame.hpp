#ifndef ca_system_trace_frame_hpp
#define ca_system_trace_frame_hpp

#include "1-primitives/debug/source_location.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	/* @DOC Error traces: where an error came from and what each caller was doing. ca::trace::fail
	   starts one, ca::trace::context adds a step, ca::trace::of reads it, ca::print_trace prints
	   it; ca::report keeps one beyond the thread, with formatted messages. */
	namespace trace {


		// -- FRAME ----------------------------------------------------------

		/* @DOC One step of a trace: what was being done (may be empty), and where. */
		class frame final {


			private:

				// -- private types ------------------------------------------

				using self = ca::trace::frame;


				// -- private members ----------------------------------------

				ca::sv::str _message;
				ca::source_location _where;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr frame(void) noexcept
				: _message{}, _where{} {
				}

				constexpr frame(const ca::sv::str message, const ca::source_location where) noexcept
				: _message{message}, _where{where} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto message(void) const noexcept -> ca::sv::str {
					return _message;
				}

				[[nodiscard]] constexpr auto where(void) const noexcept -> ca::source_location {
					return _where;
				}

		}; // class frame

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_frame_hpp
