#ifndef ca_collections_map_item_hpp
#define ca_collections_map_item_hpp

#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"
#include "5-collections/map/impl/key.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ITEM ---------------------------------------------------------------

	namespace impl {

		/* @DOC An item of K and V can clone: a part has clone(), and each part has clone() or a
		   copy constructor. */
		template <typename K, typename V>
		concept is_clonable_item = (ca::is_clonable<K> || ca::is_clonable<V>)
			&& (ca::is_clonable<K> || ca::is_copy_constructible<K>) && (ca::is_clonable<V> || ca::is_copy_constructible<V>);

	} // namespace impl


	/* @DOC A key and its value: Python's (key, value) pair, as dict.items() yields it. Owned
	   (item<K, V>) where a map stores or hands one out (popitem), borrowed (item<const K&, V&>)
	   where it is walked; structured bindings take it apart: `for (auto [key, value] : map)`.
	   Cleanable or clonable when a part is. */
	template <typename K, typename V>
	struct item final {

			// -- private types ----------------------------------------------

			using self = ca::item<K, V>;


			// -- public constants -------------------------------------------

			/* @DOC A byte copy moves it when it moves both parts. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<K> && ca::is_trivially_relocatable<V>;


			// -- public members ---------------------------------------------

			K key; // member-prefix: ignore
			V value; // member-prefix: ignore


			// -- methods ----------------------------------------------------

			/* @DOC Cleans up both parts; returns the first error. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void>
			requires (!ca::is_reference<K> && !ca::is_reference<V> && (ca::is_cleanable<K> || ca::is_cleanable<V>)) {

				ca::error first{};
				ca::impl::clean(key, first);
				ca::impl::clean(value, first);
				if (!first.ok())
					return ca::failure{first};
				return ca::result<void>{};
			}

			/* @DOC A copy, each part by clone() when it has one, else by its copy constructor. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self>
			requires (!ca::is_reference<K> && !ca::is_reference<V> && ca::impl::is_clonable_item<K, V>) {

				auto cloned_key = ca::impl::make_clone(key);
				if (!cloned_key)
					return cloned_key.propagate();
				auto cloned_value = ca::impl::make_clone(value);
				if (!cloned_value) {
					ca::error ignored{};
					ca::impl::clean(*cloned_key, ignored);
					return cloned_value.propagate();
				}
				return self{ca::move(*cloned_key), ca::move(*cloned_value)};
			}

	}; // struct item

} // namespace ca

#endif // ca_collections_map_item_hpp
