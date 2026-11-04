/* @DOC Performance of ca::basic_regex<A>.

   Layout: the program (instructions of 16 bytes, byte-range transitions of 8, a guard word per
   instruction), the group names, the pattern, the prefix literal, and one scratch block for the
   matcher, all from A. Scratch per thread key: two sparse-set entries and two lists of slot
   words (2 per group, group 0 included, plus one per loop that can iterate empty); keys are the
   instructions, twice that when such a loop exists. The program is bounded by 2^18 instructions
   and transitions, so a regex takes at most a few MiB.

   | Operation | Time | Notes |
   |---|---|---|
   | make | O(pattern + program) | parse, tree, NFA, analyses, one scratch allocation |
   | search, match, fullmatch | O(text * program) worst case | each byte read once; threads deduplicated per position |
   | literal-only pattern | O(text) | one substring search, the VM never runs |
   | skip to a match start | O(text) | the literal every match starts with (substring search), else a 256-bit first-byte set |
   | a match's groups | O(groups) | one allocation from A when the pattern has groups, none otherwise |
   | findall, split, sub | O(text * program) | the scratch reused; only the result allocates |
   | clone | as make | the pattern is compiled again |

   No pattern backtracks: (a|b|c)*d, (a*)*b and their kin take linear time where a backtracker
   takes quadratic or exponential time. The price is a constant per text byte: every live thread
   advances one byte in turn, binary-searching its transitions past 8, so a match covering most
   of the text (\w+ over prose) costs more per byte than a backtracker that runs straight
   through it. A lazy DFA for match bounds, with the VM kept for groups, would remove that cost.

   Locality: instructions and transitions are contiguous arrays walked by index; the thread lists
   are dense arrays in priority order with per-key slot rows, cleared in O(1).

   Copy and move: the copy constructor is deleted, clone() recompiles; moves take the pointers,
   noexcept. Matching methods are not const (they use the scratch): one thread per regex.

   Baseline std::regex (ECMAScript, libstdc++'s backtracker):
   --BENCH bench/regex.cpp
   bench/regex.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | compile ([a-z]+)@([a-z]+)\.com, 1e4 | 0.028x | 0.028x | ca: parse, tree, NFA, analysis, scratch; std: its NFA |
   | missing 16-char literal in 256 KiB, 4 times | 0.007x | 0.007x | ca: no VM, one substring search |
   | \w+ over 256 KiB, every match | 1.24x | 1.30x | slower: the VM steps every byte of each long match, std runs straight through |
   | [a-z]+ing\b over 256 KiB, every match | 0.90x | 0.92x | ca: one pass per match attempt, no backtracking |
   | ([a-z]+)@([a-z]+)\.com over 256 KiB, groups | 0.90x | 1.04x | ca: captures in the VM threads |
   | a repeated 3-way alternation then d, 4 KiB without d, 20 times | 0.000x | 0.000x | std backtracks from every position: quadratic; ca: linear |
   | sub(\s+, _) of 256 KiB | 0.41x | 0.44x | |
   | split(,) of 256 KiB | 0.063x | 0.067x | ca: literal pattern, no VM |
   BENCH-- */

#ifndef ca_collections_regex_basic_regex_hpp
#define ca_collections_regex_basic_regex_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/invoke_result.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_invocable.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/fail.hpp"
#include "4-lifecycle/memory/allocate_n.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/deallocate_n.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/array/slice.hpp"
#include "5-collections/regex/impl/pike_vm.hpp"
#include "5-collections/regex/impl/regex_analysis.hpp"
#include "5-collections/regex/impl/regex_compiler.hpp"
#include "5-collections/regex/impl/regex_names.hpp"
#include "5-collections/regex/impl/regex_parser.hpp"
#include "5-collections/regex/impl/regex_program.hpp"
#include "5-collections/regex/impl/regex_replacement.hpp"
#include "5-collections/regex/impl/regex_tree.hpp"
#include "5-collections/regex/regex_error.hpp"
#include "5-collections/regex/regex_flags.hpp"
#include "5-collections/regex/regex_match.hpp"
#include "5-collections/regex/regex_matches.hpp"
#include "5-collections/regex/regex_pattern.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BASIC REGEX --------------------------------------------------------

	namespace impl {

		/* @DOC The groups of the match in a matcher's best slots, as views of the text. */
		struct regex_slots_view final {

				using self = ca::impl::regex_slots_view;

				const char*    _text;
				const ca::usz* _slots;

				[[nodiscard]] auto operator[](const ca::u32 group) const noexcept -> ca::sv::str {

					const ca::usz first = _slots[2U * group];
					if (first == ca::impl::regex_no_position)
						return ca::sv::str{};
					return ca::sv::str{_text + first, _slots[2U * group + 1U] - first};
				}

		}; // struct regex_slots_view

	} // namespace impl

	/* @DOC A compiled regular expression with Python's re API, in memory from A: make (or
	   compile) from a regex_pattern and regex_flags, then search, match, fullmatch, finditer,
	   findall, split, sub; escape quotes text. Syntax is Python's (see regex_pattern), matching
	   linear in the text whatever the pattern: a Thompson NFA over UTF-8 bytes run as a Pike
	   VM, leftmost-first like Python's backtracker, so no backreferences or lookaround.
	   Positions count code units; invalid UTF-8 in a text matches no character. The matching
	   methods use the regex's own scratch memory (taken by make, so they never allocate but to
	   return results): one thread at a time per regex, clone() for another. Clean up with
	   cleanup(). */
	template <ca::is_allocator A>
	class basic_regex final {


		private:

			// -- friends ----------------------------------------------------

			friend class ca::regex_matches<ca::basic_regex<A>>;


			// -- private types ----------------------------------------------

			using self = ca::basic_regex<A>;


		public:

			// -- public types -----------------------------------------------

			using allocator_type = A;
			using match_type     = ca::regex_match<A>;
			using matches_type   = ca::regex_matches<ca::basic_regex<A>>;
			using string_type    = ca::basic_str<char, A>;
			using views_type     = ca::dynamic_array<ca::sv::str, A>;

			/* @DOC The same regex type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::basic_regex<B>;


		private:

			// -- private members --------------------------------------------

			ca::dynamic_array<ca::impl::regex_inst, A>       _insts;
			ca::dynamic_array<ca::impl::regex_transition, A> _transitions;
			ca::dynamic_array<ca::u32, A>                    _guards;
			ca::dynamic_array<ca::impl::regex_name, A>       _names;
			ca::basic_str<char, A>                           _name_text;
			ca::basic_str<char, A>                           _pattern;
			ca::basic_str<char, A>                           _prefix;
			ca::allocation                                   _block;
			ca::impl::regex_scratch                          _scratch;
			ca::u64                                          _first_bytes[4];
			ca::u32                                          _start;
			ca::u32                                          _groups;
			ca::u32                                          _slots;
			ca::regex_flags                                  _flags;
			bool                                             _skips;
			bool                                             _anchored;
			bool                                             _literal;
			[[no_unique_address]] A                          _allocator;


			// -- private lifecycle ------------------------------------------

			explicit basic_regex(const A& allocator) noexcept
			: _insts{allocator}, _transitions{allocator}, _guards{allocator}, _names{allocator}, _name_text{allocator}, _pattern{allocator}, _prefix{allocator},
			  _block{nullptr, 0U}, _scratch{}, _first_bytes{}, _start{0U}, _groups{0U}, _slots{2U}, _flags{ca::regex_flags::none}, _skips{false}, _anchored{false},
			  _literal{false}, _allocator{allocator} {
			}


		public:

			// -- public lifecycle -------------------------------------------

			basic_regex(const self&) = delete;

			basic_regex(self&& other) noexcept
			: _insts{ca::move(other._insts)}, _transitions{ca::move(other._transitions)}, _guards{ca::move(other._guards)}, _names{ca::move(other._names)},
			  _name_text{ca::move(other._name_text)},
			  _pattern{ca::move(other._pattern)}, _prefix{ca::move(other._prefix)}, _block{ca::exchange(other._block, ca::allocation{nullptr, 0U})},
			  _scratch{other._scratch}, _first_bytes{other._first_bytes[0], other._first_bytes[1], other._first_bytes[2], other._first_bytes[3]},
			  _start{other._start}, _groups{other._groups}, _slots{other._slots}, _flags{other._flags}, _skips{other._skips}, _anchored{other._anchored},
			  _literal{other._literal},
			  _allocator{other._allocator} {
			}

			/* @DOC pattern compiled under flags, in memory from allocator. errc::invalid_argument
			   with the syntax error's message as the trace (regex_pattern::check gives its
			   position), or for a program past the size limit; errc::out_of_memory. */
			[[nodiscard]] static auto make_in(const A& allocator, const ca::regex_pattern pattern, const ca::regex_flags flags = ca::regex_flags::none) noexcept -> ca::result<self> {

				self regex{allocator};
				auto built = regex._build(pattern.text(), flags);
				if (!built) {
					static_cast<void>(regex.cleanup());
					return built.propagate();
				}
				return regex;
			}

			[[nodiscard]] static auto make(const ca::regex_pattern pattern, const ca::regex_flags flags = ca::regex_flags::none) noexcept -> ca::result<self>
			requires ca::is_stateless_allocator<A> {
				return self::make_in(A{}, pattern, flags);
			}

			/* @DOC make: Python's re.compile. */
			[[nodiscard]] static auto compile(const ca::regex_pattern pattern, const ca::regex_flags flags = ca::regex_flags::none) noexcept -> ca::result<self>
			requires ca::is_stateless_allocator<A> {
				return self::make_in(A{}, pattern, flags);
			}

			/* @DOC text with every character special to a pattern backslashed: Python's
			   re.escape. */
			[[nodiscard]] static auto escape_in(const A& allocator, const ca::sv::str text) noexcept -> ca::result<string_type> {

				string_type out{allocator};
				const ca::sv::str special{"()[]{}?*+-|^$\\.&~# \t\n\r\v\f"};
				for (const char c : text) {
					auto pushed = special.find(c) ? out.push('\\') : ca::result<void>{};
					if (pushed)
						pushed = out.push(c);
					if (!pushed) {
						static_cast<void>(out.cleanup());
						return pushed.propagate();
					}
				}
				return out;
			}

			[[nodiscard]] static auto escape(const ca::sv::str text) noexcept -> ca::result<string_type> requires ca::is_stateless_allocator<A> {
				return self::escape_in(A{}, text);
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's program; the previous one is released as by cleanup(). */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				static_cast<void>(this->cleanup());
				      _insts = ca::move(other._insts);
				_transitions = ca::move(other._transitions);
				     _guards = ca::move(other._guards);
				      _names = ca::move(other._names);
				  _name_text = ca::move(other._name_text);
				    _pattern = ca::move(other._pattern);
				     _prefix = ca::move(other._prefix);
				      _block = ca::exchange(other._block, ca::allocation{nullptr, 0U});
				    _scratch = other._scratch;
				for (ca::usz i = 0U; i < 4U; ++i)
					_first_bytes[i] = other._first_bytes[i];
				     _start = other._start;
				    _groups = other._groups;
				     _slots = other._slots;
				     _flags = other._flags;
				     _skips = other._skips;
				  _anchored = other._anchored;
				   _literal = other._literal;
				 _allocator = other._allocator;
				return *this;
			}


			// -- accessors --------------------------------------------------

			/* @DOC The pattern's text: Python's Pattern.pattern. */
			[[nodiscard]] auto pattern(void) const noexcept -> ca::sv::str {
				return _pattern.view();
			}

			/* @DOC The flags given to make (inline ones not included). */
			[[nodiscard]] auto flags(void) const noexcept -> ca::regex_flags {
				return _flags;
			}

			/* @DOC Capture groups: Python's Pattern.groups. */
			[[nodiscard]] auto group_count(void) const noexcept -> ca::usz {
				return _groups;
			}

			/* @DOC The index of the group named name, or nothing: Python's Pattern.groupindex. */
			[[nodiscard]] auto group_index(const ca::sv::str name) const noexcept -> ca::optional<ca::usz> {

				const ca::optional<ca::u32> index = this->_group_names().find(name);
				if (!index)
					return ca::nullopt;
				return static_cast<ca::usz>(*index);
			}

			[[nodiscard]] auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}


			// -- methods ----------------------------------------------------

			/* @DOC The first match in text from pos on, or nothing: Python's Pattern.search. `^`
			   and `\b` still see the text before pos; pass a prefix of text for Python's
			   endpos. Fails only out of memory (a match's groups). */
			[[nodiscard]] auto search(const ca::sv::str text, const ca::usz pos = 0U) noexcept -> ca::result<ca::optional<match_type>> {

				if (!this->_find(text, pos, ca::impl::regex_no_position, false, false))
					return ca::optional<match_type>{};
				return this->_matched(text);
			}

			/* @DOC A match starting at pos, or nothing: Python's Pattern.match. */
			[[nodiscard]] auto match(const ca::sv::str text, const ca::usz pos = 0U) noexcept -> ca::result<ca::optional<match_type>> {

				if (!this->_find(text, pos, ca::impl::regex_no_position, true, false))
					return ca::optional<match_type>{};
				return this->_matched(text);
			}

			/* @DOC A match from pos to the end of text, or nothing: Python's Pattern.fullmatch. */
			[[nodiscard]] auto fullmatch(const ca::sv::str text, const ca::usz pos = 0U) noexcept -> ca::result<ca::optional<match_type>> {

				if (!this->_find(text, pos, ca::impl::regex_no_position, true, true))
					return ca::optional<match_type>{};
				return this->_matched(text);
			}

			/* @DOC The matches in text from pos on, found one by one by next(): Python's
			   Pattern.finditer. Non-overlapping; an empty match is allowed right after a
			   non-empty one but never twice at one position. Borrows the regex and text. */
			[[nodiscard]] auto finditer(const ca::sv::str text, const ca::usz pos = 0U) noexcept -> matches_type {
				return matches_type{*this, text, pos};
			}

			/* @DOC Python's Pattern.findall: for each match, the whole match when the pattern has
			   no group, the group when it has one, every group in turn when it has more (the
			   tuples flattened). A group that took no part is an empty view. */
			[[nodiscard]] auto findall(const ca::sv::str text, const ca::usz pos = 0U) noexcept -> ca::result<views_type> {

				views_type found{_allocator};
				ca::usz at = this->_clamp(text, pos);
				ca::usz forbid = ca::impl::regex_no_position;
				while (this->_next(text, at, forbid)) {
					const ca::impl::regex_slots_view groups{text.data(), _scratch._best};
					ca::result<void> added{};
					if (_groups == 0U)
						added = found.append(groups[0U]);
					for (ca::u32 group = 1U; group <= _groups && added; ++group)
						added = found.append(groups[group]);
					if (!added) {
						static_cast<void>(found.cleanup());
						return added.propagate();
					}
				}
				return found;
			}

			/* @DOC text cut at the matches, at most max times (0: no limit), the groups' texts
			   between the pieces (empty for a group that took no part): Python's Pattern.split.
			   Empty matches cut too. */
			[[nodiscard]] auto split(const ca::sv::str text, const ca::usz max = 0U) noexcept -> ca::result<views_type> {

				views_type pieces{_allocator};
				ca::usz at = 0U;
				ca::usz forbid = ca::impl::regex_no_position;
				ca::usz last = 0U;
				ca::usz count = 0U;
				ca::result<void> added{};
				while ((max == 0U || count < max) && added && this->_next(text, at, forbid)) {
					const ca::impl::regex_slots_view groups{text.data(), _scratch._best};
					added = pieces.append(ca::sv::str{text.data() + last, _scratch._best[0] - last});
					for (ca::u32 group = 1U; group <= _groups && added; ++group)
						added = pieces.append(groups[group]);
					last = _scratch._best[1];
					++count;
				}
				if (added)
					added = pieces.append(ca::sv::str{text.data() + last, text.size() - last});
				if (!added) {
					static_cast<void>(pieces.cleanup());
					return added.propagate();
				}
				return pieces;
			}

			/* @DOC text with its first count matches (0: all) replaced by replacement, its group
			   references filled in (`\1`, `\g<1>`, `\g<name>`, escapes as Python reads them):
			   Python's Pattern.sub. errc::invalid_argument for a bad template, with a trace. */
			[[nodiscard]] auto sub(const ca::sv::str replacement, const ca::sv::str text, const ca::usz count = 0U) noexcept -> ca::result<string_type> {

				ca::impl::regex_template_check check{};
				const ca::optional<ca::sv::str> problem = ca::impl::regex_expand(replacement, _groups, this->_group_names(), check);
				if (problem)
					return ca::trace::fail(ca::errc::invalid_argument, *problem);
				return this->_substitute(text, count, [this, replacement](string_type& out, const ca::sv::str searched) noexcept -> bool {
					const ca::impl::regex_slots_view groups{searched.data(), _scratch._best};
					ca::impl::regex_appender<ca::impl::regex_slots_view, A> appender{out, groups};
					static_cast<void>(ca::impl::regex_expand(replacement, _groups, this->_group_names(), appender));
					return appender.ok();
				});
			}

			/* @DOC sub with the replacement of each match computed by replace(match), returning
			   text (a view the call must keep alive until it returns). */
			template <typename F>
			requires ca::is_invocable<F&, const ca::regex_match<A>&> && ca::is_convertible<ca::invoke_result<F&, const ca::regex_match<A>&>, ca::sv::str>
			[[nodiscard]] auto sub(F&& replace, const ca::sv::str text, const ca::usz count = 0U) noexcept -> ca::result<string_type> {
				return this->_substitute(text, count, [this, &replace](string_type& out, const ca::sv::str searched) noexcept -> bool {
					auto found = this->_matched(searched);
					if (!found)
						return false;
					ca::optional<match_type>& match = *found;
					if (!match)
						return false;
					const bool appended = static_cast<bool>(out.append(static_cast<ca::sv::str>(replace(static_cast<const match_type&>(*match)))));
					static_cast<void>(match->cleanup());
					return appended;
				});
			}

			/* @DOC Another regex of the same pattern and flags, in memory from the same
			   allocator: one per thread. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> {
				return self::make_in(_allocator, ca::regex_pattern{_pattern.view()}, _flags);
			}

			/* @DOC Frees the program, the names and the scratch memory. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				static_cast<void>(_insts.cleanup());
				static_cast<void>(_transitions.cleanup());
				static_cast<void>(_guards.cleanup());
				static_cast<void>(_names.cleanup());
				static_cast<void>(_name_text.cleanup());
				static_cast<void>(_pattern.cleanup());
				static_cast<void>(_prefix.cleanup());
				if (_block.data() != nullptr)
					ca::deallocate_n<ca::usz>(_allocator, _block);
				  _block = ca::allocation{nullptr, 0U};
				_scratch = ca::impl::regex_scratch{};
				return ca::result<void>{};
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard]] auto _group_names(void) const noexcept -> ca::impl::regex_names {
				return ca::impl::regex_names{_names.view(), _name_text.view()};
			}

			[[nodiscard]] static auto _clamp(const ca::sv::str text, const ca::usz pos) noexcept -> ca::usz {
				return pos > text.size() ? text.size() : pos;
			}

			[[nodiscard]] auto _program(void) const noexcept -> ca::impl::regex_program {
				return ca::impl::regex_program{_insts.data(), _transitions.data(), _guards.data(), _prefix.data(), _prefix.size(), static_cast<ca::u32>(_insts.size()), _start,
					_slots, this->_shift(), {_first_bytes[0], _first_bytes[1], _first_bytes[2], _first_bytes[3]}, _skips, _anchored};
			}

			[[nodiscard]] auto _shift(void) const noexcept -> ca::u32 {
				return _slots > 2U * _groups + 2U ? 1U : 0U;
			}

			[[nodiscard]] auto _find_literal(const ca::sv::str text, const ca::usz from, const bool anchored, const bool full) noexcept -> bool {

				const ca::sv::str literal = _prefix.view();
				ca::usz start = from;
				if (anchored) {
					if (!text.without_prefix(from).starts_with(literal))
						return false;
				}
				else {
					const ca::optional<ca::usz> hit = text.without_prefix(from).find(literal);
					if (!hit)
						return false;
					start = from + *hit;
				}
				if (full && start + literal.size() != text.size())
					return false;
				_scratch._best[0] = start;
				_scratch._best[1] = start + literal.size();
				return true;
			}

			[[nodiscard]] auto _find(const ca::sv::str text, const ca::usz pos, const ca::usz forbid, const bool anchored, const bool full) noexcept -> bool {

				const ca::usz from = self::_clamp(text, pos);
				if (_literal)
					return this->_find_literal(text, from, anchored, full);
				const ca::impl::regex_program program = this->_program();
				return ca::impl::regex_execute(program, _scratch, ca::impl::regex_search{text.data(), text.size(), from, forbid, anchored, full});
			}

			/* @DOC The next match from at, past the empty one at forbid; moves at and forbid on. */
			[[nodiscard]] auto _next(const ca::sv::str text, ca::usz& at, ca::usz& forbid) noexcept -> bool {

				if (at > text.size() || !this->_find(text, at, forbid, false, false))
					return false;
				const ca::usz start = _scratch._best[0];
				const ca::usz end = _scratch._best[1];
				forbid = start == end ? end : ca::impl::regex_no_position;
				    at = end;
				return true;
			}

			[[nodiscard]] auto _matched(const ca::sv::str text) noexcept -> ca::result<ca::optional<match_type>> {

				ca::slice<ca::usz, A> groups{_allocator};
				if (_groups != 0U) {
					auto made = ca::slice<ca::usz, A>::make_in(_allocator, ca::borrowed_slice<const ca::usz>{_scratch._best + 2U, 2U * static_cast<ca::usz>(_groups)});
					if (!made)
						return made.propagate();
					groups = ca::move(*made);
				}
				return ca::optional<match_type>{match_type{text, this->_group_names(), _scratch._best[0], _scratch._best[1], ca::move(groups)}};
			}

			template <typename Append>
			[[nodiscard]] auto _substitute(const ca::sv::str text, const ca::usz count, Append&& append) noexcept -> ca::result<string_type> {

				string_type out{_allocator};
				ca::usz at = 0U;
				ca::usz forbid = ca::impl::regex_no_position;
				ca::usz last = 0U;
				ca::usz done = 0U;
				bool ok = true;
				while (ok && (count == 0U || done < count) && this->_next(text, at, forbid)) {
					const ca::usz start = _scratch._best[0];
					const ca::usz end = _scratch._best[1];
					  ok = out.append(ca::sv::str{text.data() + last, start - last}) && append(out, text);
					last = end;
					++done;
				}
				if (ok)
					ok = static_cast<bool>(out.append(ca::sv::str{text.data() + last, text.size() - last}));
				if (!ok) {
					static_cast<void>(out.cleanup());
					return ca::failure{ca::errc::out_of_memory};
				}
				return out;
			}

			[[nodiscard]] auto _build(const ca::sv::str pattern, const ca::regex_flags flags) noexcept -> ca::result<void> {

				_flags = flags;
				auto copied = _pattern.assign(pattern);
				if (!copied)
					return copied;
				ca::impl::regex_tree<A> tree{_allocator};
				auto compiled = this->_compile(pattern, flags, tree);
				tree.cleanup();
				if (!compiled)
					return compiled;
				auto skips = ca::impl::regex_first_bytes(_allocator, _insts, _transitions, _start, _first_bytes);
				if (!skips)
					return skips.propagate();
				   _skips = *skips;
				_anchored = ca::impl::regex_anchored(_insts, _start);
				return this->_reserve_scratch();
			}

			[[nodiscard]] auto _compile(const ca::sv::str pattern, const ca::regex_flags flags, ca::impl::regex_tree<A>& tree) noexcept -> ca::result<void> {

				ca::impl::regex_parser<ca::impl::regex_tree<A>> parser{pattern, tree};
				const ca::optional<ca::regex_error> problem = parser.run(flags);
				if (!tree.ok())
					return ca::failure{ca::errc::out_of_memory};
				if (problem)
					return ca::trace::fail(ca::errc::invalid_argument, problem->message());
				   _groups = parser.groups();
				    _names = tree.take_names();
				_name_text = tree.take_name_text();
				bool whole = true;
				bool ok = true;
				const bool complete = ca::impl::regex_prefix(tree, tree.top(), _prefix, whole, ok);
				if (!ok)
					return ca::failure{ca::errc::out_of_memory};
				_literal = complete && whole && !_prefix.empty();
				ca::impl::regex_compiler<A> compiler{_allocator, tree, _groups, _insts, _transitions, _guards};
				auto start = compiler.run();
				if (!start) {
					if (start.error() == ca::errc::invalid_argument)
						return ca::trace::fail(ca::errc::invalid_argument, ca::regex_error::message_of(ca::regex_error::kind::too_large));
					return start.propagate();
				}
				_start = *start;
				_slots = compiler.slots();
				return ca::result<void>{};
			}

			[[nodiscard]] auto _reserve_scratch(void) noexcept -> ca::result<void> {

				const ca::usz keys = _insts.size() << this->_shift();
				ca::usz bytes{0U};
				if (!ca::impl::regex_scratch_size(keys, _slots, bytes))
					return ca::failure{ca::errc::out_of_memory};
				auto block = ca::allocate_n<ca::usz>(_allocator, (bytes + sizeof(ca::usz) - 1U) / sizeof(ca::usz));
				if (!block)
					return block.propagate();
				  _block = *block;
				_scratch = ca::impl::regex_scratch_at(_block.data(), keys, _slots);
				return ca::result<void>{};
			}

	}; // class basic_regex

} // namespace ca

#endif // ca_collections_regex_basic_regex_hpp
