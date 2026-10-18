/* @DOC ca::report: an error and its trace, owned.
   - Layout: the error, a dropped count, one basic_str<char, A> holding every message end to
     end, and a dynamic_array<A> of frames (offset and size in the text, source location):
     two allocations whatever the number of frames, none when the messages fit the string's
     inline capacity and no frame was added.
   - Cost: failure path only. of() copies the thread trace's messages (one append each);
     context() formats in place at the end of the text, then pushes one frame. Both amortized
     O(message size).
   - Never fails: a frame that does not fit in memory is counted in dropped() instead.
   - frame(i) views the text: valid until the next context().
*/
#ifndef ca_collections_trace_report_hpp
#define ca_collections_trace_report_hpp

#include "1-primitives/debug/source_location.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/type_identity.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/frame.hpp"
#include "3-system/trace/impl/display_trace.hpp"
#include "3-system/trace/of.hpp"
#include "3-system/trace/record.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/format/format_append.hpp"
#include "5-collections/format/is_displayable.hpp"
#include "5-collections/format/located_format_string.hpp"
#include "5-collections/format/text_writer.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REPORT -------------------------------------------------------------

	namespace impl {

		struct report_frame final {

				using self = ca::impl::report_frame;

				ca::usz _begin;
				ca::usz _size;
				ca::source_location _where;

		}; // struct report_frame

	} // namespace impl


	/* @DOC An error and its trace, owned, in memory from A: what to return as E once an error
	   leaves the thread or needs formatted context. of(e) copies the thread's trace of e;
	   context("reading {}", path) adds a frame here (the caller); see also ca::context. Prints
	   with `{}` as ca::trace::record. Clean up with destroy(); the destructor is a failsafe. */
	template <ca::is_allocator A = ca::heap>
	class report final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::report<A>;


		public:

			// -- public types -----------------------------------------------

			using allocator_type = A;

			/* @DOC The same report type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::report<B>;


		private:

			// -- private members --------------------------------------------

			ca::error _error;
			ca::usz _dropped;
			ca::basic_str<char, A> _text;
			ca::dynamic_array<ca::impl::report_frame, A> _frames;


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC error, no frame. */
			explicit report(const ca::error error) noexcept requires ca::is_stateless_allocator<A>
			: _error{error}, _dropped{0U}, _text{}, _frames{} {
			}

			report(const A& allocator, const ca::error error) noexcept
			: _error{error}, _dropped{0U}, _text{allocator}, _frames{allocator} {
			}

			report(const self&) = delete;

			report(self&&) noexcept = default;

			/* @DOC Failsafe for a skipped destroy(): frees, silently. */
			~report(void) noexcept = default;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			auto operator=(self&&) noexcept -> self& = default;


			// -- factories --------------------------------------------------

			/* @DOC error with a copy of its thread trace (ca::trace::of), when it has one. */
			[[nodiscard, gnu::cold]] static auto of(const ca::error error) noexcept -> self requires ca::is_stateless_allocator<A> {

				self report{error};
				report._capture();
				return report;
			}

			[[nodiscard, gnu::cold]] static auto of(const A& allocator, const ca::error error) noexcept -> self {

				self report{allocator, error};
				report._capture();
				return report;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] auto error(void) const noexcept -> ca::error {
				return _error;
			}

			/* @DOC Frames kept, origin first. */
			[[nodiscard]] auto size(void) const noexcept -> ca::usz {
				return _frames.size();
			}

			/* @DOC The index-th frame, its message a view of the report's text. Traps out of range. */
			[[nodiscard]] auto frame(const ca::usz index) const noexcept -> ca::trace::frame {

				const ca::impl::report_frame& entry = _frames[index];
				return ca::trace::frame{ca::sv::str{_text.data() + entry._begin, entry._size}, entry._where};
			}

			/* @DOC Frames lost: past the thread trace's capacity, or out of memory. */
			[[nodiscard]] auto dropped(void) const noexcept -> ca::usz {
				return _dropped;
			}


			// -- methods ----------------------------------------------------

			/* @DOC Adds a frame here (the caller), its message formatted (`{}` syntax, checked at
			   compile time); counted in dropped() when memory runs out. */
			template <typename... Args>
			requires (ca::is_displayable<Args> && ...)
			[[gnu::cold]] auto context(const ca::located_format_string<ca::type_identity<Args>...> format, const Args&... args) noexcept -> void {

				const ca::usz begin = _text.size();
				auto appended = ca::format_append(_text, format.format(), args...);
				if (!appended) [[unlikely]] {
					++_dropped;
					return;
				}
				this->_push(begin, format.where());
			}

			auto display(ca::text_writer& out) const noexcept -> void {
				ca::trace::impl::display_trace(out, _error, _frames.size(), _dropped, [this](const ca::usz i) noexcept {
					return this->frame(i);
				});
			}

			/* @DOC Frees the text and the frames; the error stays. */
			[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {

				auto text = _text.destroy();
				auto frames = _frames.destroy();
				if (!text) [[unlikely]]
					return text;
				return frames;
			}


		private:

			// -- private methods --------------------------------------------

			auto _capture(void) noexcept -> void {

				const ca::trace::record record = ca::trace::of(_error);
				_dropped += record.dropped();
				for (const ca::trace::frame& frame : record.frames()) {
					const ca::usz begin = _text.size();
					auto appended = _text.append(frame.message());
					if (!appended) [[unlikely]] {
						++_dropped;
						continue;
					}
					this->_push(begin, frame.where());
				}
			}

			auto _push(const ca::usz begin, const ca::source_location where) noexcept -> void {

				const ca::impl::report_frame entry{begin, _text.size() - begin, where};
				auto added = _frames.append(entry);
				if (!added) [[unlikely]] {
					_text.set_size(begin);
					++_dropped;
				}
			}

	}; // class report

} // namespace ca

#endif // ca_collections_trace_report_hpp
