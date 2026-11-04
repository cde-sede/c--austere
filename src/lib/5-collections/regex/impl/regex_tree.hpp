#ifndef ca_collections_regex_impl_regex_tree_hpp
#define ca_collections_regex_impl_regex_tree_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/regex/impl/regex_limits.hpp"
#include "5-collections/regex/impl/regex_look.hpp"
#include "5-collections/regex/impl/regex_node.hpp"
#include "5-collections/regex/impl/regex_parser.hpp"
#include "5-collections/regex/impl/regex_program.hpp"
#include "5-collections/regex/impl/regex_ranges.hpp"
#include "5-collections/regex/regex_flags.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX TREE ---------------------------------------------------------

	namespace impl {

		/* @DOC The parser's sink that builds the pattern's tree in memory from A: nodes linked by
		   index, every set resolved to sorted ranges (named classes, case folding and negation
		   applied), group names. A failed allocation turns ok() false, which stops the parse. */
		template <ca::is_allocator A>
		class regex_tree final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::regex_tree<A>;


			private:

				// -- private members ----------------------------------------

				ca::dynamic_array<ca::impl::regex_node, A>  _nodes;
				ca::dynamic_array<ca::impl::regex_range, A> _ranges;
				ca::dynamic_array<ca::impl::regex_range, A> _set;
				ca::dynamic_array<ca::impl::regex_name, A>  _names;
				ca::basic_str<char, A>                      _name_text;
				ca::u32                                     _root;
				bool                                        _ok;


			public:

				// -- public lifecycle ---------------------------------------

				explicit regex_tree(const A& allocator) noexcept
				: _nodes{allocator}, _ranges{allocator}, _set{allocator}, _names{allocator}, _name_text{allocator}, _root{0U}, _ok{true} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] auto ok(void) const noexcept -> bool {
					return _ok;
				}

				[[nodiscard]] auto node(const ca::u32 index) const noexcept -> const ca::impl::regex_node& {
					return _nodes[index];
				}

				[[nodiscard]] auto ranges(void) const noexcept -> const ca::impl::regex_range* {
					return _ranges.data();
				}

				[[nodiscard]] auto top(void) const noexcept -> ca::u32 {
					return _root;
				}

				/* @DOC The names, handed over (the tree keeps none). */
				[[nodiscard]] auto take_names(void) noexcept -> ca::dynamic_array<ca::impl::regex_name, A> {
					return ca::move(_names);
				}

				[[nodiscard]] auto take_name_text(void) noexcept -> ca::basic_str<char, A> {
					return ca::move(_name_text);
				}


				// -- methods ------------------------------------------------

				[[nodiscard]] auto empty(void) noexcept -> ca::u32 {
					return this->_add(ca::impl::regex_node_kind::empty, 0U, 0U, ca::impl::regex_none);
				}

				[[nodiscard]] auto literal(const char32_t point, const ca::regex_flags flags) noexcept -> ca::u32 {

					if (!ca::impl::has_flag(flags, ca::regex_flags::ignore_case))
						return this->_add(ca::impl::regex_node_kind::literal, static_cast<ca::u32>(point), 0U, ca::impl::regex_none);
					this->set_begin();
					this->_check(ca::impl::regex_append_caseless(point, ca::impl::has_flag(flags, ca::regex_flags::ascii), _set));
					if (_ok && _set.size() == 1U)
						return this->_add(ca::impl::regex_node_kind::literal, static_cast<ca::u32>(point), 0U, ca::impl::regex_none);
					return this->set_end(false, ca::regex_flags::none);
				}

				[[nodiscard]] auto any(const ca::regex_flags flags) noexcept -> ca::u32 {

					this->set_begin();
					if (!ca::impl::has_flag(flags, ca::regex_flags::dot_all))
						this->set_range(U'\n', U'\n');
					return this->set_end(true, ca::regex_flags::none);
				}

				[[nodiscard]] auto look(const ca::impl::regex_look kind) noexcept -> ca::u32 {
					return this->_add(ca::impl::regex_node_kind::look, static_cast<ca::u32>(kind), 0U, ca::impl::regex_none);
				}

				auto set_begin(void) noexcept -> void {
					this->_check(_set.clear());
				}

				auto set_range(const char32_t first, const char32_t last) noexcept -> void {
					this->_check(_set.append(ca::impl::regex_range{first, last}));
				}

				auto set_class(const ca::impl::regex_class which, const bool negated, const ca::regex_flags flags) noexcept -> void {
					this->_check(ca::impl::regex_append_class(which, negated, ca::impl::has_flag(flags, ca::regex_flags::ascii), _set));
				}

				[[nodiscard]] auto set_end(const bool negated, const ca::regex_flags flags) noexcept -> ca::u32 {

					if (!_ok)
						return 0U;
					this->_check(ca::impl::regex_normalize(_set));
					if (_ok && ca::impl::has_flag(flags, ca::regex_flags::ignore_case))
						this->_check(ca::impl::regex_fold(_set, ca::impl::has_flag(flags, ca::regex_flags::ascii)));
					if (!_ok)
						return 0U;
					const ca::usz first = _ranges.size();
					if (negated)
						this->_check(ca::impl::regex_append_complement(_set.view(), _ranges));
					else
						this->_check(_ranges.extend(_set.view()));
					return this->_add(ca::impl::regex_node_kind::set, static_cast<ca::u32>(first), static_cast<ca::u32>(_ranges.size() - first), ca::impl::regex_none);
				}

				[[nodiscard]] auto repeat(const ca::u32 child, const ca::u32 min, const ca::u32 max, const bool greedy) noexcept -> ca::u32 {

					const ca::u32 index = this->_add(ca::impl::regex_node_kind::repeat, min, max, child);
					if (_ok)
						_nodes[index]._greedy = greedy;
					return index;
				}

				[[nodiscard]] auto group(const ca::u32 child, const ca::u32 capture) noexcept -> ca::u32 {
					return this->_add(ca::impl::regex_node_kind::group, capture, 0U, child);
				}

				[[nodiscard]] auto name(const ca::sv::str name, const ca::u32 capture) noexcept -> bool {

					for (const ca::impl::regex_name& known : _names) {
						if (ca::sv::str{_name_text.data() + known._name_at, known._name_size} == name)
							return false;
					}
					const ca::usz at = _name_text.size();
					this->_check(_name_text.append(name));
					this->_check(_names.append(ca::impl::regex_name{static_cast<ca::u32>(at), static_cast<ca::u32>(name.size()), capture}));
					return true;
				}

				[[nodiscard]] auto concat(const ca::u32 first) noexcept -> ca::u32 {
					return this->_add(ca::impl::regex_node_kind::concat, 0U, 0U, first);
				}

				[[nodiscard]] auto alternate(const ca::u32 first) noexcept -> ca::u32 {
					return this->_add(ca::impl::regex_node_kind::alternate, 0U, 0U, first);
				}

				auto link(const ca::u32 previous, const ca::u32 next) noexcept -> void {
					if (_ok)
						_nodes[previous]._next = next;
				}

				auto root(const ca::u32 handle) noexcept -> void {
					_root = handle;
				}

				/* @DOC Frees what the tree still owns. */
				auto cleanup(void) noexcept -> void {

					static_cast<void>(_nodes.cleanup());
					static_cast<void>(_ranges.cleanup());
					static_cast<void>(_set.cleanup());
					static_cast<void>(_names.cleanup());
					static_cast<void>(_name_text.cleanup());
				}


			private:

				// -- private methods ----------------------------------------

				auto _check(const ca::result<void>& done) noexcept -> void {
					if (!done)
						_ok = false;
				}

				[[nodiscard]] auto _add(const ca::impl::regex_node_kind kind, const ca::u32 a, const ca::u32 b, const ca::u32 child) noexcept -> ca::u32 {

					if (!_ok)
						return 0U;
					const ca::usz index = _nodes.size();
					this->_check(_nodes.append(ca::impl::regex_node{child, ca::impl::regex_none, a, b, kind, true}));
					return _ok ? static_cast<ca::u32>(index) : 0U;
				}

		}; // class regex_tree

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_tree_hpp
