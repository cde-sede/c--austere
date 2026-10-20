#ifndef ca_system_trace_record_hpp
#define ca_system_trace_record_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/backtrace.hpp"
#include "3-system/trace/frame.hpp"
#include "3-system/trace/impl/display_trace.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- RECORD ---------------------------------------------------------

		/* @DOC An error and the frames of its trace, origin first, and the backtrace of the first
		   trap it met, borrowed from the thread's trace: valid until the thread starts another
		   one. Prints with `{}`:
		   ```
		   No such file or directory
		     at open_config (app.cpp:12): opening config
		     at load (app.cpp:40): loading settings
		   ``` */
		class record final {


			private:

				// -- private types ------------------------------------------

				using self = ca::trace::record;


				// -- private members ----------------------------------------

				ca::error _error;
				ca::borrowed_slice<const ca::trace::frame> _frames;
				ca::usz _dropped;
				const ca::trace::backtrace* _backtrace;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr record(const ca::error error, const ca::borrowed_slice<const ca::trace::frame> frames, const ca::usz dropped,
				                 const ca::trace::backtrace* const backtrace = nullptr) noexcept
				: _error{error}, _frames{frames}, _dropped{dropped}, _backtrace{backtrace} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto error(void) const noexcept -> ca::error {
					return _error;
				}

				[[nodiscard]] constexpr auto frames(void) const noexcept -> ca::borrowed_slice<const ca::trace::frame> {
					return _frames;
				}

				/* @DOC Frames past the thread's capacity, lost. */
				[[nodiscard]] constexpr auto dropped(void) const noexcept -> ca::usz {
					return _dropped;
				}

				/* @DOC The calls leading to the first trap the trace met (see ca::trace::backtrace),
				   empty when none. */
				[[nodiscard]] constexpr auto backtrace(void) const noexcept -> ca::borrowed_slice<const ca::usz> {
					return _backtrace == nullptr ? ca::borrowed_slice<const ca::usz>{} : _backtrace->addresses();
				}


				// -- methods ------------------------------------------------

				template <typename W>
				auto display(W& out) const noexcept -> void {

					ca::trace::impl::display_trace(out, _error, _frames.size(), _dropped, [this](const ca::usz i) noexcept {
						return _frames[i];
					});
					if (_backtrace != nullptr && !_backtrace->empty()) {
						out.put(ca::sv::str{"\n  backtrace:"});
						_backtrace->display(out);
					}
				}

		}; // class record

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_record_hpp
