#ifndef ca_collections_map_is_map_hpp
#define ca_collections_map_is_map_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS MAP -------------------------------------------------------------

	/* @DOC Keys mapped to values, Python's dict as ca spells it; what hashmap and map share.
	   - key_type, mapped_type; lookup_type, what keys are looked up by (an sv for str keys).
	   - size(), empty(), contains(key): len and in.
	   - map[key]: the value; traps when key is absent, in every build (KeyError).
	   - get(key) -> optional<mapped_type&>: the value, allowing the doubt.
	   - set(key, value) -> result<void>: d[key] = value, inserting or replacing; may allocate.
	   - pop(key) -> optional<mapped_type>: removes and returns the value.
	   - clear(), cleanup() -> result<void>: cleanup of every entry, then of the storage.
	   - begin(), end(): every entry once, as item<const key_type&, mapped_type&>. */
	template <typename M>
	concept is_map = requires(M& map, const M& view, const typename M::lookup_type key, typename M::mapped_type&& value) {
		typename M::key_type;
		typename M::mapped_type;
		{ view.size() } -> ca::is_same<ca::usz>;
		{ view.empty() } -> ca::is_same<bool>;
		{ view.contains(key) } -> ca::is_same<bool>;
		{ map[key] } -> ca::is_same<typename M::mapped_type&>;
		{ view[key] } -> ca::is_same<const typename M::mapped_type&>;
		{ map.get(key) } -> ca::is_same<ca::optional<typename M::mapped_type&>>;
		{ view.get(key) } -> ca::is_same<ca::optional<const typename M::mapped_type&>>;
		{ map.set(key, ca::move(value)) } -> ca::is_same<ca::result<void>>;
		{ map.pop(key) } -> ca::is_same<ca::optional<typename M::mapped_type>>;
		{ map.clear() } -> ca::is_same<ca::result<void>>;
		{ map.cleanup() } -> ca::is_same<ca::result<void>>;
		map.begin() != map.end();
		view.begin() != view.end();
	};

} // namespace ca

#endif // ca_collections_map_is_map_hpp
