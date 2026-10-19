#ifndef ca_system_trace_impl_display_trace_hpp
#define ca_system_trace_impl_display_trace_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/error_message.hpp"
#include "3-system/trace/frame.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DISPLAY TRACE ------------------------------------------------------

	namespace trace {


		namespace impl {

			template <typename W>
			auto put_decimal(W& out, ca::usz value) noexcept -> void {

				char digits[20];
				char* at = digits + sizeof(digits);
				do {
					*--at = static_cast<char>('0' + value % 10U);
					value /= 10U;
				} while (value != 0U);
				out.put(ca::sv::str{at, static_cast<ca::usz>(digits + sizeof(digits) - at)});
			}

			/* @DOC The error's message, then one line per frame, origin first:
			   `  at function (file:line): message` (`  trapped: message` for a frame with no
			   location: a trap left for a context), and how many frames were dropped. W has
			   put(char) and put(sv::str); frame_at(i) returns the i-th ca::trace::frame. */
			template <typename W, typename F>
			auto display_trace(W& out, const ca::error error, const ca::usz count, const ca::usz dropped, const F& frame_at) noexcept -> void {

				out.put(ca::sys::error_message(error));
				for (ca::usz i = 0U; i < count; ++i) {
					const ca::trace::frame frame = frame_at(i);
					if (frame.where().line() == 0U) {
						out.put(ca::sv::str{"\n  trapped: "});
						out.put(frame.message());
						continue;
					}
					out.put(ca::sv::str{"\n  at "});
					out.put(ca::sv::str{frame.where().function()});
					out.put(ca::sv::str{" ("});
					out.put(ca::sv::str{frame.where().file()});
					out.put(':');
					ca::trace::impl::put_decimal(out, frame.where().line());
					out.put(')');
					if (!frame.message().empty()) {
						out.put(ca::sv::str{": "});
						out.put(frame.message());
					}
				}
				if (dropped != 0U) {
					out.put(ca::sv::str{"\n  ... "});
					ca::trace::impl::put_decimal(out, dropped);
					out.put(ca::sv::str{dropped == 1U ? " more frame" : " more frames"});
				}
			}

		} // namespace impl

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_impl_display_trace_hpp
