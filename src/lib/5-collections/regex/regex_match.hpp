#ifndef ca_collections_regex_regex_match_hpp
#define ca_collections_regex_regex_match_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/array/slice.hpp"
#include "5-collections/regex/impl/pike_vm.hpp"
#include "5-collections/regex/impl/regex_names.hpp"
#include "5-collections/regex/impl/regex_replacement.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX MATCH --------------------------------------------------------

	namespace impl {

		/* @DOC Appends a template's expansion to a string; the first failed append is kept. */
		template <typename Groups, ca::is_allocator A>
		class regex_appender final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::regex_appender<Groups, A>;


			private:

				// -- private members ----------------------------------------

				ca::basic_str<char, A>& _out;
				const Groups&           _groups;
				bool                    _ok;


			public:

				// -- public lifecycle ---------------------------------------

				regex_appender(ca::basic_str<char, A>& out, const Groups& groups) noexcept
				: _out{out}, _groups{groups}, _ok{true} {
				}


				// -- methods ------------------------------------------------

				[[nodiscard]] auto ok(void) const noexcept -> bool {
					return _ok;
				}

				auto text(const ca::sv::str piece) noexcept -> void {
					if (_ok && !_out.append(piece))
						_ok = false;
				}

				auto point(const char32_t point) noexcept -> void {
					if (_ok && !_out.append_code_point(point))
						_ok = false;
				}

				auto group(const ca::u32 index) noexcept -> void {
					this->text(_groups[index]);
				}

		}; // class regex_appender

		/* @DOC The template sink that checks only. */
		struct regex_template_check final {

				using self = ca::impl::regex_template_check;

				constexpr auto text(const ca::sv::str) noexcept -> void {
				}

				constexpr auto point(const char32_t) noexcept -> void {
				}

				constexpr auto group(const ca::u32) noexcept -> void {
				}

		}; // struct regex_template_check

	} // namespace impl

	/* @DOC One match of a basic_regex in a text: Python's re.Match. Group 0 is the whole match,
	   groups 1 to group_count() the pattern's capture groups, positions in code units of the
	   searched text. It borrows that text and the regex's group names (both must outlive it)
	   and owns the groups' positions, from A when the pattern has groups. operator[] traps on
	   a group past group_count() and gives an empty view for a group that took no part; group,
	   start and end give nothing then. Clean up with cleanup(). */
	template <ca::is_allocator A = ca::heap>
	class regex_match final {


		private:

			// -- friends ----------------------------------------------------

			template <ca::is_allocator B>
			friend class basic_regex; // qualify: ignore


			// -- private types ----------------------------------------------

			using self = ca::regex_match<A>;


		public:

			// -- public types -----------------------------------------------

			using allocator_type = A;
			using string_type    = ca::basic_str<char, A>;

			/* @DOC The same match type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::regex_match<B>;


		private:

			// -- private members --------------------------------------------

			ca::sv::str             _text;
			ca::impl::regex_names   _names;
			ca::usz                 _start;
			ca::usz                 _end;
			ca::slice<ca::usz, A>   _groups;


			// -- private lifecycle ------------------------------------------

			regex_match(const ca::sv::str text, const ca::impl::regex_names names, const ca::usz start, const ca::usz end, ca::slice<ca::usz, A>&& groups) noexcept
			: _text{text}, _names{names}, _start{start}, _end{end}, _groups{ca::move(groups)} {
			}


		public:

			// -- public lifecycle -------------------------------------------

			regex_match(const self&) = delete;

			regex_match(self&& other) noexcept = default;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			auto operator=(self&& other) noexcept -> self& = default;


			// -- accessors --------------------------------------------------

			/* @DOC The text searched: Python's Match.string. */
			[[nodiscard]] auto string(void) const noexcept -> ca::sv::str {
				return _text;
			}

			/* @DOC The pattern's capture groups, group 0 not counted. */
			[[nodiscard]] auto group_count(void) const noexcept -> ca::usz {
				return _groups.size() / 2U;
			}

			[[nodiscard]] auto start(void) const noexcept -> ca::usz {
				return _start;
			}

			[[nodiscard]] auto end(void) const noexcept -> ca::usz {
				return _end;
			}

			/* @DOC Where group starts, or nothing when it took no part or is past group_count(). */
			[[nodiscard]] auto start(const ca::usz group) const noexcept -> ca::optional<ca::usz> {

				if (!this->_participates(group))
					return ca::nullopt;
				return group == 0U ? _start : _groups.data()[2U * group - 2U];
			}

			[[nodiscard]] auto end(const ca::usz group) const noexcept -> ca::optional<ca::usz> {

				if (!this->_participates(group))
					return ca::nullopt;
				return group == 0U ? _end : _groups.data()[2U * group - 1U];
			}


			// -- element access ---------------------------------------------

			/* @DOC Text of group; empty when it took no part. Traps when group > group_count(). */
			[[nodiscard]] auto operator[](const ca::usz group) const noexcept -> ca::sv::str {

				if (group > this->group_count()) [[unlikely]]
					ca::impl::index_out_of_bounds();
				if (!this->_participates(group))
					return ca::sv::str{};
				return this->_slice(group);
			}

			/* @DOC Text of the group named name; traps when no group has that name. */
			[[nodiscard]] auto operator[](const ca::sv::str name) const noexcept -> ca::sv::str {

				const ca::optional<ca::u32> index = _names.find(name);
				if (!index) [[unlikely]]
					ca::impl::index_out_of_bounds();
				return (*this)[*index];
			}

			/* @DOC The whole match. */
			[[nodiscard]] auto group(void) const noexcept -> ca::sv::str {
				return ca::sv::str{_text.data() + _start, _end - _start};
			}

			/* @DOC Text of group, or nothing when it took no part or is past group_count(). */
			[[nodiscard]] auto group(const ca::usz group) const noexcept -> ca::optional<ca::sv::str> {

				if (!this->_participates(group))
					return ca::nullopt;
				return this->_slice(group);
			}

			/* @DOC Text of the group named name, or nothing when it took no part or no group has
			   that name. */
			[[nodiscard]] auto group(const ca::sv::str name) const noexcept -> ca::optional<ca::sv::str> {

				const ca::optional<ca::u32> index = _names.find(name);
				if (!index)
					return ca::nullopt;
				return this->group(static_cast<ca::usz>(*index));
			}

			/* @DOC The index of the group named name, or nothing. */
			[[nodiscard]] auto group_index(const ca::sv::str name) const noexcept -> ca::optional<ca::usz> {

				const ca::optional<ca::u32> index = _names.find(name);
				if (!index)
					return ca::nullopt;
				return static_cast<ca::usz>(*index);
			}


			// -- methods ----------------------------------------------------

			/* @DOC replacement with its group references filled in (`\1`, `\g<name>`, as
			   basic_regex::sub reads it): Python's Match.expand. errc::invalid_argument for a
			   bad template, with a trace. */
			[[nodiscard]] auto expand(const ca::sv::str replacement) const noexcept -> ca::result<string_type> {

				string_type out{_groups.allocator()};
				const ca::optional<ca::sv::str> problem = this->expand_into(replacement, out);
				if (problem) {
					static_cast<void>(out.cleanup());
					return ca::trace::fail(ca::errc::invalid_argument, *problem);
				}
				return out;
			}

			/* @DOC Appends expand(replacement) to out: nothing, or what went wrong, a literal (the
			   template's error, or "out of memory" when out could not grow; what fit stays). */
			[[nodiscard]] auto expand_into(const ca::sv::str replacement, string_type& out) const noexcept -> ca::optional<ca::sv::str> {

				ca::impl::regex_template_check check{};
				const ca::optional<ca::sv::str> problem = ca::impl::regex_expand(replacement, static_cast<ca::u32>(this->group_count()), _names, check);
				if (problem)
					return problem;
				ca::impl::regex_appender<self, A> appender{out, *this};
				static_cast<void>(ca::impl::regex_expand(replacement, static_cast<ca::u32>(this->group_count()), _names, appender));
				if (!appender.ok())
					return ca::sv::str{"out of memory"};
				return ca::nullopt;
			}

			/* @DOC A copy in memory from the same allocator. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> {

				auto groups = _groups.clone();
				if (!groups)
					return groups.propagate();
				return self{_text, _names, _start, _end, ca::move(*groups)};
			}

			/* @DOC Frees the groups' positions. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {
				return _groups.cleanup();
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard]] auto _participates(const ca::usz group) const noexcept -> bool {

				if (group == 0U)
					return true;
				return group <= this->group_count() && _groups.data()[2U * group - 2U] != ca::impl::regex_no_position;
			}

			[[nodiscard]] auto _slice(const ca::usz group) const noexcept -> ca::sv::str {

				const ca::usz first = group == 0U ? _start : _groups.data()[2U * group - 2U];
				const ca::usz last = group == 0U ? _end : _groups.data()[2U * group - 1U];
				return ca::sv::str{_text.data() + first, last - first};
			}

	}; // class regex_match

} // namespace ca

#endif // ca_collections_regex_regex_match_hpp
