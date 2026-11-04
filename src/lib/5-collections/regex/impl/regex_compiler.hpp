#ifndef ca_collections_regex_impl_regex_compiler_hpp
#define ca_collections_regex_impl_regex_compiler_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/map/hashmap.hpp"
#include "5-collections/regex/impl/regex_limits.hpp"
#include "5-collections/regex/impl/regex_node.hpp"
#include "5-collections/regex/impl/regex_program.hpp"
#include "5-collections/regex/impl/regex_ranges.hpp"
#include "5-collections/regex/impl/regex_tree.hpp"
#include "5-collections/regex/impl/utf8_sequences.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX COMPILER -----------------------------------------------------

	namespace impl {

		/* @DOC Compiles a tree into a Thompson NFA over UTF-8 bytes, each node knowing what follows
		   it (nothing to patch). A set becomes a trie of byte ranges, its UTF-8 sequences merged
		   by prefix and identical states shared; counted repeats are unrolled. Fails with
		   errc::out_of_memory, or errc::invalid_argument when the program passes
		   regex_max_program. */
		template <ca::is_allocator A>
		class regex_compiler final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::regex_compiler<A>;


			private:

				// -- private members ----------------------------------------

				const ca::impl::regex_tree<A>&                     _tree;
				ca::dynamic_array<ca::impl::regex_inst, A>&        _insts;
				ca::dynamic_array<ca::impl::regex_transition, A>&  _transitions;
				ca::dynamic_array<ca::u32, A>&                     _guards;
				ca::dynamic_array<ca::u32, A>                      _work;
				ca::dynamic_array<ca::impl::regex_transition, A>   _levels[4];
				ca::hashmap<ca::u64, ca::u32, A>                   _shared;
				ca::u32                                            _slots;
				ca::u32                                            _guard;
				bool                                               _out_of_memory;
				bool                                               _too_large;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC Slots from groups' up are the loops' own (progress); guards gets each
				   instruction's innermost such loop slot, or regex_none. */
				regex_compiler(const A& allocator, const ca::impl::regex_tree<A>& tree, const ca::u32 groups, ca::dynamic_array<ca::impl::regex_inst, A>& insts,
					ca::dynamic_array<ca::impl::regex_transition, A>& transitions, ca::dynamic_array<ca::u32, A>& guards) noexcept
				: _tree{tree}, _insts{insts}, _transitions{transitions}, _guards{guards}, _work{allocator},
				  _levels{ca::dynamic_array<ca::impl::regex_transition, A>{allocator}, ca::dynamic_array<ca::impl::regex_transition, A>{allocator},
					ca::dynamic_array<ca::impl::regex_transition, A>{allocator}, ca::dynamic_array<ca::impl::regex_transition, A>{allocator}},
				  _shared{allocator}, _slots{2U * groups + 2U}, _guard{ca::impl::regex_none}, _out_of_memory{false}, _too_large{false} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC Slots per thread: the groups', then one per loop that can iterate empty. */
				[[nodiscard]] auto slots(void) const noexcept -> ca::u32 {
					return _slots;
				}


				// -- methods ------------------------------------------------

				/* @DOC The entry instruction of the whole tree, ending in one match. */
				[[nodiscard]] auto run(void) noexcept -> ca::result<ca::u32> {

					const ca::u32 match = this->_op(ca::impl::regex_op::match, 0U, 0U);
					const ca::u32 start = this->_compile(_tree.top(), match);
					this->_release();
					if (_out_of_memory)
						return ca::failure{ca::errc::out_of_memory};
					if (_too_large)
						return ca::failure{ca::errc::invalid_argument};
					return start;
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] auto _failed(void) const noexcept -> bool {
					return _out_of_memory || _too_large;
				}

				auto _check(const ca::result<void>& done) noexcept -> void {
					if (!done)
						_out_of_memory = true;
				}

				auto _release(void) noexcept -> void {

					static_cast<void>(_work.cleanup());
					for (ca::dynamic_array<ca::impl::regex_transition, A>& level : _levels)
						static_cast<void>(level.cleanup());
					static_cast<void>(_shared.cleanup());
				}

				[[nodiscard]] auto _emit(const ca::impl::regex_inst inst) noexcept -> ca::u32 {

					if (this->_failed())
						return 0U;
					if (_insts.size() + _transitions.size() >= ca::impl::regex_max_program) {
						_too_large = true;
						return 0U;
					}
					const ca::usz pc = _insts.size();
					this->_check(_insts.append(inst));
					this->_check(_guards.append(_guard));
					return static_cast<ca::u32>(pc);
				}

				[[nodiscard]] auto _op(const ca::impl::regex_op op, const ca::u32 x, const ca::u32 y, const ca::u32 z = 0U, const ca::u8 look = 0U) noexcept -> ca::u32 {
					return this->_emit(ca::impl::regex_inst{x, y, z, op, look});
				}

				[[nodiscard]] auto _split(const ca::u32 first, const ca::u32 second) noexcept -> ca::u32 {
					return this->_op(ca::impl::regex_op::split, first, second);
				}

				[[nodiscard]] auto _byte(const ca::u8 low, const ca::u8 high, const ca::u32 next) noexcept -> ca::u32 {

					if (this->_failed())
						return 0U;
					const ca::usz at = _transitions.size();
					this->_check(_transitions.append(ca::impl::regex_transition{next, low, high}));
					return this->_op(ca::impl::regex_op::bytes, static_cast<ca::u32>(at), 1U);
				}

				[[nodiscard]] auto _nullable(const ca::u32 index) const noexcept -> bool {

					const ca::impl::regex_node& node = _tree.node(index);
					switch (node._kind) {
						case ca::impl::regex_node_kind::empty:
						case ca::impl::regex_node_kind::look:
							return true;
						case ca::impl::regex_node_kind::literal:
						case ca::impl::regex_node_kind::set:
							return false;
						case ca::impl::regex_node_kind::group:
							return this->_nullable(node._child);
						case ca::impl::regex_node_kind::repeat:
							return node._a == 0U || this->_nullable(node._child);
						case ca::impl::regex_node_kind::concat:
							for (ca::u32 child = node._child; child != ca::impl::regex_none; child = _tree.node(child)._next) {
								if (!this->_nullable(child))
									return false;
							}
							return true;
						case ca::impl::regex_node_kind::alternate:
							for (ca::u32 child = node._child; child != ca::impl::regex_none; child = _tree.node(child)._next) {
								if (this->_nullable(child))
									return true;
							}
							return false;
					}
					return false;
				}

				/* @DOC One iteration of node's child that leaves for exit when it matched empty and
				   goes on to more otherwise (slot holds where it began). */
				[[nodiscard]] auto _checked(const ca::impl::regex_node& node, const ca::u32 slot, const ca::u32 more, const ca::u32 exit) noexcept -> ca::u32 {

					const ca::u32 outer = _guard;
					_guard = slot;
					const ca::u32 check = this->_op(ca::impl::regex_op::progress, exit, slot, more);
					const ca::u32 body = this->_compile(node._child, check);
					_guard = outer;
					return this->_op(ca::impl::regex_op::save, body, slot);
				}

				[[nodiscard]] auto _compile(const ca::u32 index, const ca::u32 next) noexcept -> ca::u32 {

					if (this->_failed())
						return 0U;
					const ca::impl::regex_node node = _tree.node(index);
					switch (node._kind) {
						case ca::impl::regex_node_kind::empty:
							return next;
						case ca::impl::regex_node_kind::literal:
							return this->_literal(static_cast<char32_t>(node._a), next);
						case ca::impl::regex_node_kind::set:
							return this->_set(node._a, node._b, next);
						case ca::impl::regex_node_kind::look:
							return this->_op(ca::impl::regex_op::look, next, 0U, 0U, static_cast<ca::u8>(node._a));
						case ca::impl::regex_node_kind::group: {
							const ca::u32 close = this->_op(ca::impl::regex_op::save, next, 2U * node._a + 1U);
							const ca::u32 body = this->_compile(node._child, close);
							return this->_op(ca::impl::regex_op::save, body, 2U * node._a);
						}
						case ca::impl::regex_node_kind::repeat:
							return this->_repeat(node, next);
						case ca::impl::regex_node_kind::concat:
							return this->_concat(node._child, next);
						case ca::impl::regex_node_kind::alternate:
							return this->_alternate(node._child, next);
					}
					return next;
				}

				[[nodiscard]] auto _literal(const char32_t point, const ca::u32 next) noexcept -> ca::u32 {

					ca::impl::utf8_sequences sequences{point, point};
					ca::impl::utf8_sequence sequence{};
					static_cast<void>(sequences.next(sequence));
					ca::u32 pc = next;
					for (ca::usz i = sequence._length; i != 0U; --i)
						pc = this->_byte(sequence._low[i - 1U], sequence._high[i - 1U], pc);
					return pc;
				}

				[[nodiscard]] auto _children(const ca::u32 first) noexcept -> ca::usz {

					const ca::usz base = _work.size();
					for (ca::u32 child = first; child != ca::impl::regex_none && !this->_failed(); child = _tree.node(child)._next)
						this->_check(_work.append(child));
					return base;
				}

				[[nodiscard]] auto _concat(const ca::u32 first, const ca::u32 next) noexcept -> ca::u32 {

					const ca::usz base = this->_children(first);
					ca::u32 pc = next;
					for (ca::usz i = _work.size(); i > base && !this->_failed(); --i)
						pc = this->_compile(_work[i - 1U], pc);
					this->_check(_work.resize(base));
					return pc;
				}

				[[nodiscard]] auto _alternate(const ca::u32 first, const ca::u32 next) noexcept -> ca::u32 {

					const ca::usz base = this->_children(first);
					const ca::usz end = _work.size();
					for (ca::usz i = base; i < end && !this->_failed(); ++i)
						_work[i] = this->_compile(_work[i], next);
					ca::u32 pc = this->_failed() ? 0U : _work[end - 1U];
					for (ca::usz i = end - 1U; i > base && !this->_failed(); --i)
						pc = this->_split(_work[i - 1U], pc);
					this->_check(_work.resize(base));
					return pc;
				}

				[[nodiscard]] auto _loop(const ca::impl::regex_node& node, const ca::u32 next, const bool at_least_once, const ca::u32 slot) noexcept -> ca::u32 {

					const ca::u32 loop = this->_split(0U, 0U);
					const ca::u32 body = slot == ca::impl::regex_none ? this->_compile(node._child, loop) : this->_checked(node, slot, loop, next);
					if (this->_failed())
						return 0U;
					_insts[loop]._x = node._greedy ? body : next;
					_insts[loop]._y = node._greedy ? next : body;
					return at_least_once ? body : loop;
				}

				/* @DOC min copies, then the rest: a loop (unbounded) or max - min optional copies.
				   A child that can match empty is iterated as Python's re does: past min, an
				   iteration that matched empty ends the repeat, its groups kept. */
				[[nodiscard]] auto _repeat(const ca::impl::regex_node& node, const ca::u32 next) noexcept -> ca::u32 {

					const ca::u32 min = node._a;
					const ca::u32 max = node._b;
					if (max == 0U)
						return next;
					const bool nullable = this->_nullable(node._child);
					const ca::u32 slot = nullable ? _slots++ : ca::impl::regex_none;
					ca::u32 pc = next;
					ca::u32 copies = min;
					if (max == ca::impl::regex_infinite) {
						const bool merged = min != 0U && !nullable;
						pc = this->_loop(node, next, merged, slot);
						if (merged)
							--copies;
					}
					else {
						for (ca::u32 i = min; i < max && !this->_failed(); ++i) {
							const ca::u32 body = nullable ? this->_checked(node, slot, pc, next) : this->_compile(node._child, pc);
							pc = node._greedy ? this->_split(body, next) : this->_split(next, body);
						}
					}
					for (ca::u32 i = 0U; i < copies && !this->_failed(); ++i)
						pc = this->_compile(node._child, pc);
					return pc;
				}

				[[nodiscard]] auto _set(const ca::u32 first, const ca::u32 count, const ca::u32 next) noexcept -> ca::u32 {

					const ca::impl::regex_range* const ranges = _tree.ranges() + first;
					if (count == 0U || ranges[count - 1U]._last < 0x80U) {
						const ca::usz at = _transitions.size();
						for (ca::u32 i = 0U; i < count && !this->_failed(); ++i)
							this->_check(_transitions.append(ca::impl::regex_transition{next, static_cast<ca::u8>(ranges[i]._first), static_cast<ca::u8>(ranges[i]._last)}));
						return this->_op(ca::impl::regex_op::bytes, static_cast<ca::u32>(at), count);
					}
					this->_check(_shared.clear());
					ca::usz depth = 0U;
					for (ca::u32 i = 0U; i < count && !this->_failed(); ++i) {
						ca::impl::utf8_sequences sequences{ranges[i]._first, ranges[i]._last};
						ca::impl::utf8_sequence sequence{};
						while (sequences.next(sequence) && !this->_failed())
							depth = this->_add_sequence(sequence, depth, next);
					}
					for (ca::usz i = depth; i > 1U && !this->_failed(); --i)
						this->_levels[i - 2U][this->_levels[i - 2U].size() - 1U]._next = this->_close(i - 1U);
					return this->_close(0U);
				}

				[[nodiscard]] auto _add_sequence(const ca::impl::utf8_sequence& sequence, const ca::usz depth, const ca::u32 next) noexcept -> ca::usz {

					const ca::usz length = sequence._length;
					ca::usz common = 0U;
					while (common + 1U < depth && common + 1U < length) {
						const ca::impl::regex_transition& last = _levels[common][_levels[common].size() - 1U];
						if (last._low != sequence._low[common] || last._high != sequence._high[common])
							break;
						++common;
					}
					for (ca::usz i = depth; i > common + 1U && !this->_failed(); --i)
						_levels[i - 2U][_levels[i - 2U].size() - 1U]._next = this->_close(i - 1U);
					for (ca::usz i = common; i < length && !this->_failed(); ++i)
						this->_check(_levels[i].append(ca::impl::regex_transition{i + 1U == length ? next : ca::impl::regex_none, sequence._low[i], sequence._high[i]}));
					return length;
				}

				[[nodiscard]] static auto _hash(const ca::impl::regex_transition* const transitions, const ca::usz count) noexcept -> ca::u64 {

					ca::u64 hash = 0xCBF29CE484222325U;
					for (ca::usz i = 0U; i < count; ++i) {
						const ca::u64 value = (static_cast<ca::u64>(transitions[i]._next) << 16U) | (static_cast<ca::u64>(transitions[i]._low) << 8U) | transitions[i]._high;
						hash = (hash ^ value) * 0x100000001B3U;
					}
					return hash;
				}

				[[nodiscard]] auto _same(const ca::u32 pc, const ca::impl::regex_transition* const transitions, const ca::usz count) const noexcept -> bool {

					const ca::impl::regex_inst& inst = _insts[pc];
					if (inst._y != count)
						return false;
					for (ca::usz i = 0U; i < count; ++i) {
						const ca::impl::regex_transition& known = _transitions[inst._x + i];
						if (known._next != transitions[i]._next || known._low != transitions[i]._low || known._high != transitions[i]._high)
							return false;
					}
					return true;
				}

				[[nodiscard]] auto _close(const ca::usz level) noexcept -> ca::u32 {

					ca::dynamic_array<ca::impl::regex_transition, A>& open = _levels[level];
					const ca::u64 hash = self::_hash(open.data(), open.size());
					const ca::optional<ca::u32&> known = _shared.get(hash);
					if (known && this->_same(*known, open.data(), open.size())) {
						this->_check(open.clear());
						return *known;
					}
					const ca::usz at = _transitions.size();
					this->_check(_transitions.extend(open.view()));
					const ca::u32 pc = this->_op(ca::impl::regex_op::bytes, static_cast<ca::u32>(at), static_cast<ca::u32>(open.size()));
					if (!known && !this->_failed())
						this->_check(_shared.set(hash, pc));
					this->_check(open.clear());
					return pc;
				}

		}; // class regex_compiler

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_compiler_hpp
