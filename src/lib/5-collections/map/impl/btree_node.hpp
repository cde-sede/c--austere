#ifndef ca_collections_map_impl_btree_node_hpp
#define ca_collections_map_impl_btree_node_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cv.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "5-collections/map/item.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {


		// -- BTREE NODE -----------------------------------------------------

		/* @DOC A B-tree node: up to CAPACITY sorted keys and their values in two arrays (key
		   scans touch keys only), its parent and its index there. One more slot of each lets an
		   insertion overflow a full node before it splits. A branch adds the children. */
		template <typename K, typename V> // final: ignore
		struct btree_node {

				using self = ca::impl::btree_node<K, V>;

				static constexpr ca::usz CAPACITY = 11U;
				static constexpr ca::usz MINIMUM  = CAPACITY / 2U;

				self*   _parent;
				ca::u16 _index;
				ca::u16 _size;
				bool    _branch;
				union {
						K _keys[CAPACITY + 1U];
				};
				union {
						V _values[CAPACITY + 1U];
				};

				constexpr explicit btree_node(const bool branch) noexcept
				: _parent{nullptr}, _index{0U}, _size{0U}, _branch{branch} {
				}

				btree_node(const self&) = delete;

				~btree_node(void) noexcept {
				}

				auto operator=(const self&) -> self& = delete;

		}; // struct btree_node


		/* @DOC A node with children: child i holds the keys between keys i - 1 and i. */
		template <typename K, typename V>
		struct btree_branch final : ca::impl::btree_node<K, V> {

				using self = ca::impl::btree_branch<K, V>;

				using node = ca::impl::btree_node<K, V>;

				node* _children[node::CAPACITY + 2U];

				constexpr btree_branch(void) noexcept
				: node{true} {
				}

		}; // struct btree_branch


		/* @DOC child of a branch node. */
		template <typename K, typename V>
		[[nodiscard, gnu::always_inline]] constexpr auto btree_child(const ca::impl::btree_node<K, V>* const at, const ca::usz index) noexcept -> ca::impl::btree_node<K, V>* {
			return static_cast<const ca::impl::btree_branch<K, V>*>(at)->_children[index];
		}


		// -- BTREE ITERATOR -------------------------------------------------

		/* @DOC Walks a map in key order, showing each entry through Project::get(key, value).
		   Positions do not survive insertions and removals. */
		template <typename K, typename V, typename Project>
		class btree_iterator final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::btree_iterator<K, V, Project>;

				using node = ca::impl::btree_node<K, ca::remove_cv<V>>;


				// -- private members ----------------------------------------

				node*   _node;
				ca::usz _index;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr btree_iterator(node* const at, const ca::usz index) noexcept
				: _node{at}, _index{index} {
				}


				// -- element access -----------------------------------------

				/* @DOC The entry, as Project shows it. Traps at end(), in every build. */
				[[nodiscard]] constexpr auto operator*(void) const noexcept -> decltype(auto) {

					if (_node == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					return Project::template get<K, V>(_node->_keys[_index], _node->_values[_index]);
				}


				// -- methods ------------------------------------------------

				/* @DOC The next entry in key order. Traps at end(), in every build. */
				constexpr auto operator++(void) noexcept -> self& {

					if (_node == nullptr) [[unlikely]]
						ca::impl::index_out_of_bounds();
					if (_node->_branch) {
						node* at = ca::impl::btree_child(_node, _index + 1U);
						while (at->_branch)
							at = ca::impl::btree_child(at, 0U);
						 _node = at;
						_index = 0U;
						return *this;
					}
					++_index;
					while (_index == _node->_size) {
						if (_node->_parent == nullptr) {
							 _node = nullptr;
							_index = 0U;
							return *this;
						}
						_index = _node->_index;
						 _node = _node->_parent;
					}
					return *this;
				}

				[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
					return _node == other._node && _index == other._index;
				}

		}; // class btree_iterator


		/* @DOC Shows a map entry as item<const K&, V&> (V const-qualified when walked read-only). */
		struct project_btree_item final {

				using self = ca::impl::project_btree_item;

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(const K& key, V& value) noexcept -> ca::item<const K&, V&> {
					return ca::item<const K&, V&>{key, value};
				}

		}; // struct project_btree_item

		/* @DOC Shows a map entry as its key. */
		struct project_btree_key final {

				using self = ca::impl::project_btree_key;

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(const K& key, V&) noexcept -> const K& {
					return key;
				}

		}; // struct project_btree_key

		/* @DOC Shows a map entry as its value. */
		struct project_btree_value final {

				using self = ca::impl::project_btree_value;

				template <typename K, typename V>
				[[nodiscard]] static constexpr auto get(const K&, V& value) noexcept -> V& {
					return value;
				}

		}; // struct project_btree_value

	} // namespace impl

} // namespace ca

#endif // ca_collections_map_impl_btree_node_hpp
