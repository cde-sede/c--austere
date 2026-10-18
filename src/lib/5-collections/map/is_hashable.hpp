#ifndef ca_collections_map_is_hashable_hpp
#define ca_collections_map_is_hashable_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_integral.hpp"
#include "1-primitives/traits/is_pointer.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "2-vocabulary/sv/basic.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {

		/* @DOC T declares hash() const -> u64. */
		template <typename T>
		concept has_hash_member = requires(const T& value) {
			{ value.hash() } -> ca::is_same<ca::u64>;
		};

		/* @DOC T is a string view, sv::basic<C>, hashed by its code units. */
		template <typename T>
		concept is_hashed_text = requires {
			typename T::value_type;
			requires ca::is_character<typename T::value_type>;
			requires ca::is_same<T, ca::sv::basic<typename T::value_type>>;
		};

		/* @DOC T owns text it shows through view() -> sv::basic<C> (the strings), hashed as that
		   view. */
		template <typename T>
		concept has_hashed_text = requires(const T& value) {
			{ value.view() } -> ca::impl::is_hashed_text;
		};

	} // namespace impl


	// -- IS HASHABLE --------------------------------------------------------

	/* @DOC Types ca::hash accepts: integers (bool and code units included), enums, pointers,
	   string views and strings (by their text), and any type declaring hash() const -> u64. A
	   type and the lookup view of it must hash alike: a str and its sv do. */
	template <typename T>
	concept is_hashable = ca::impl::has_hash_member<ca::remove_cvref<T>> || ca::is_integral<ca::remove_cvref<T>>
		|| ca::is_enum<ca::remove_cvref<T>> || ca::is_pointer<ca::remove_cvref<T>>
		|| ca::impl::is_hashed_text<ca::remove_cvref<T>> || ca::impl::has_hashed_text<ca::remove_cvref<T>>;

} // namespace ca

#endif // ca_collections_map_is_hashable_hpp
