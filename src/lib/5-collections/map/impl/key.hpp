#ifndef ca_collections_map_impl_key_hpp
#define ca_collections_map_impl_key_hpp

#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {


		// -- LOOKUP OF ------------------------------------------------------

		template <typename K>
		struct lookup_select final {

				using self = ca::impl::lookup_select<K>;

				using type = const K&;

		}; // struct lookup_select

		template <typename K>
		requires requires { typename K::view_type; } && ca::is_convertible<const K&, typename K::view_type>
		struct lookup_select<K> final {

				using self = ca::impl::lookup_select<K>;

				using type = typename K::view_type;

		}; // struct lookup_select

		/* @DOC What maps of K look keys up by: K::view_type when K shows one (an sv for a str, so
		   literals and views find string keys without building one), else const K&. */
		template <typename K>
		using lookup_of = typename ca::impl::lookup_select<K>::type;

		/* @DOC key as its lookup type. */
		template <typename K>
		[[nodiscard, gnu::always_inline]] constexpr auto key_view(const K& key) noexcept -> ca::impl::lookup_of<K> {
			return key;
		}

		/* @DOC Three-way order of two lookup keys, by sign: compare() when they have it (views),
		   else <. */
		template <typename L>
		[[nodiscard, gnu::always_inline]] constexpr auto key_order(const L& left, const L& right) noexcept -> int {

			if constexpr (requires { { left.compare(right) } -> ca::is_convertible<int>; })
				return left.compare(right);
			else
				return left < right ? -1 : (right < left ? 1 : 0);
		}


		// -- MAKE FROM ------------------------------------------------------

		/* @DOC A T can be built from a Q: by a constructor, by clone() from a T, or by a fallible
		   T::from(Q) -> result<T> (a str from a literal or a view). */
		template <typename T, typename Q>
		concept is_buildable_from = ca::is_constructible<T, Q>
			|| (ca::is_same<ca::remove_cvref<Q>, T> && ca::is_clonable<T>)
			|| requires(Q&& source) {
				{ T::from(ca::forward<Q>(source)) } -> ca::is_same<ca::result<T>>;
			};

		/* @DOC A T built from source, the cheapest way is_buildable_from allows. */
		template <typename T, typename Q>
		requires ca::impl::is_buildable_from<T, Q>
		[[nodiscard, gnu::always_inline]] inline auto make_from(Q&& source) noexcept -> ca::result<T> {

			if constexpr (ca::is_constructible<T, Q>)
				return ca::result<T>{ca::in_place, ca::forward<Q>(source)};
			else if constexpr (ca::is_same<ca::remove_cvref<Q>, T> && ca::is_clonable<T>)
				return source.clone();
			else
				return T::from(ca::forward<Q>(source));
		}

		/* @DOC A copy of value: clone() when T has one, else the copy constructor. */
		template <typename T>
		requires ca::is_clonable<T> || ca::is_copy_constructible<T>
		[[nodiscard, gnu::always_inline]] inline auto make_clone(const T& value) noexcept -> ca::result<T> {

			if constexpr (ca::is_clonable<T>)
				return value.clone();
			else
				return ca::result<T>{ca::in_place, value};
		}

		/* @DOC Runs value.destroy() when T has one, keeping the first error in first. */
		template <typename T>
		[[gnu::always_inline]] inline auto clean(T& value, ca::error& first) noexcept -> void {
			if constexpr (ca::is_cleanable<T>) {
				value.destroy().match([] {
				}, [&first](const ca::error& error) {
					if (first.ok())
						first = error;
				});
			}
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_map_impl_key_hpp
