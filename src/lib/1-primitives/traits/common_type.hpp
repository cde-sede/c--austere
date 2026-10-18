#ifndef ca_primitives_traits_common_type_hpp
#define ca_primitives_traits_common_type_hpp

#include "1-primitives/traits/decay.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/declval.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COMMON TYPE --------------------------------------------------------

	namespace impl {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wenum-compare"
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-float-conversion"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
		template <typename T, typename U>
		using ternary = decltype(false ? ca::declval<T>() : ca::declval<U>());
#pragma GCC diagnostic pop

		template <typename T, typename U>
		concept is_ternary_valid = requires {
			typename ca::impl::ternary<T, U>;
		};

		template <typename... Ts>
		struct common_type final {
				using self = ca::impl::common_type<Ts...>;
				common_type(void) = delete;
		}; // struct common_type

		template <typename T>
		requires requires {
			typename ca::impl::common_type<T, T>::type;
		}
		struct common_type<T> final {
				using self = ca::impl::common_type<T>;
				using type = typename ca::impl::common_type<T, T>::type;
				common_type(void) = delete;
		}; // struct common_type

		template <typename T, typename U>
		requires requires {
			typename ca::impl::common_type<ca::decay<T>, ca::decay<U>>::type;
		}
			&& (!(ca::is_same<T, ca::decay<T>> && ca::is_same<U, ca::decay<U>>))
		struct common_type<T, U> final {
				using self = ca::impl::common_type<T, U>;
				using type = typename ca::impl::common_type<ca::decay<T>, ca::decay<U>>::type;
				common_type(void) = delete;
		}; // struct common_type

		template <typename T, typename U>
		requires ca::impl::is_ternary_valid<T, U>
			&& ca::is_same<T, ca::decay<T>> && ca::is_same<U, ca::decay<U>>
		struct common_type<T, U> final {
				using self = ca::impl::common_type<T, U>;
				using type = ca::decay<ca::impl::ternary<T, U>>;
				common_type(void) = delete;
		}; // struct common_type

		template <typename T, typename U>
		requires ca::impl::is_ternary_valid<const T&, const U&> && (!ca::impl::is_ternary_valid<T, U>)
			&& ca::is_same<T, ca::decay<T>> && ca::is_same<U, ca::decay<U>>
		struct common_type<T, U> final {
				using self = ca::impl::common_type<T, U>;
				using type = ca::remove_cvref<ca::impl::ternary<const T&, const U&>>;
				common_type(void) = delete;
		}; // struct common_type

		template <typename T, typename U, typename V, typename... Ws>
		requires requires {
			typename ca::impl::common_type<typename ca::impl::common_type<T, U>::type, V, Ws...>::type;
		}
		struct common_type<T, U, V, Ws...> final {
				using self = ca::impl::common_type<T, U, V, Ws...>;
				using type = typename ca::impl::common_type<typename ca::impl::common_type<T, U>::type, V, Ws...>::type;
				common_type(void) = delete;
		}; // struct common_type

	} // namespace impl


	/* @DOC std::common_type_t. Customize by specializing ca::impl::common_type<A, B> on decayed types. */
	template <typename... Ts>
	using common_type = typename ca::impl::common_type<Ts...>::type;

} // namespace ca

#endif // ca_primitives_traits_common_type_hpp
