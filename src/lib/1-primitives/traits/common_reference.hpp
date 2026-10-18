#ifndef ca_primitives_traits_common_reference_hpp
#define ca_primitives_traits_common_reference_hpp

#include "1-primitives/traits/add_lvalue_reference.hpp"
#include "1-primitives/traits/add_pointer.hpp"
#include "1-primitives/traits/add_rvalue_reference.hpp"
#include "1-primitives/traits/basic_common_reference.hpp"
#include "1-primitives/traits/common_type.hpp"
#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/impl/copy_cv.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_lvalue_reference.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/traits/is_rvalue_reference.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/traits/remove_reference.hpp"
#include "1-primitives/utility/declval.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COMMON REFERENCE ---------------------------------------------------

	namespace impl {

		template <typename T, typename U>
		using copy_cvref = ca::conditional<ca::is_lvalue_reference<T>,
			ca::add_lvalue_reference<ca::impl::copy_cv<ca::remove_reference<T>, U>>,
			ca::conditional<ca::is_rvalue_reference<T>,
			ca::add_rvalue_reference<ca::impl::copy_cv<ca::remove_reference<T>, U>>,
			ca::impl::copy_cv<T, U>>>;

		template <typename T>
		struct xref final {
				using self = ca::impl::xref<T>;
				template <typename U>
				using type = ca::impl::copy_cvref<T, U>;
				xref(void) = delete;
		}; // struct xref

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wenum-compare"
#pragma GCC diagnostic ignored "-Wdeprecated"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-float-conversion"
#pragma GCC diagnostic ignored "-Wdeprecated-enum-enum-conversion"
		template <typename T, typename U>
		using cond_res = decltype(false ? ca::declval<T (&)(void)>()() : ca::declval<U (&)(void)>()());
#pragma GCC diagnostic pop

		template <typename T, typename U>
		struct common_ref final {
				using self = ca::impl::common_ref<T, U>;
				common_ref(void) = delete;
		}; // struct common_ref

		template <typename T, typename U>
		requires requires {
			typename ca::impl::cond_res<ca::impl::copy_cv<T, U>&, ca::impl::copy_cv<U, T>&>;
		}
			&& ca::is_reference<ca::impl::cond_res<ca::impl::copy_cv<T, U>&, ca::impl::copy_cv<U, T>&>>
		struct common_ref<T&, U&> final {
				using self = ca::impl::common_ref<T&, U&>;
				using type = ca::impl::cond_res<ca::impl::copy_cv<T, U>&, ca::impl::copy_cv<U, T>&>;
				common_ref(void) = delete;
		}; // struct common_ref

		template <typename T, typename U>
		requires requires {
			typename ca::impl::common_ref<T&, U&>::type;
		}
			&& ca::is_convertible<T&&, ca::remove_reference<typename ca::impl::common_ref<T&, U&>::type>&&>
			&& ca::is_convertible<U&&, ca::remove_reference<typename ca::impl::common_ref<T&, U&>::type>&&>
		struct common_ref<T&&, U&&> final {
				using self = ca::impl::common_ref<T&&, U&&>;
				using type = ca::remove_reference<typename ca::impl::common_ref<T&, U&>::type>&&;
				common_ref(void) = delete;
		}; // struct common_ref

		template <typename T, typename U>
		requires requires {
			typename ca::impl::common_ref<const T&, U&>::type;
		}
			&& ca::is_convertible<T&&, typename ca::impl::common_ref<const T&, U&>::type>
		struct common_ref<T&&, U&> final {
				using self = ca::impl::common_ref<T&&, U&>;
				using type = typename ca::impl::common_ref<const T&, U&>::type;
				common_ref(void) = delete;
		}; // struct common_ref

		template <typename T, typename U>
		requires requires {
			typename ca::impl::common_ref<U&&, T&>::type;
		}
		struct common_ref<T&, U&&> final {
				using self = ca::impl::common_ref<T&, U&&>;
				using type = typename ca::impl::common_ref<U&&, T&>::type;
				common_ref(void) = delete;
		}; // struct common_ref

		template <typename T, typename U>
		concept is_common_ref_valid = ca::is_reference<T> && ca::is_reference<U>
			&& requires {
				typename ca::impl::common_ref<T, U>::type;
			}
			&& ca::is_convertible<ca::add_pointer<T>, ca::add_pointer<typename ca::impl::common_ref<T, U>::type>>
			&& ca::is_convertible<ca::add_pointer<U>, ca::add_pointer<typename ca::impl::common_ref<T, U>::type>>;

		template <typename T, typename U>
		concept is_basic_common_ref_valid = requires {
			typename ca::basic_common_reference<ca::remove_cvref<T>, ca::remove_cvref<U>,
			ca::impl::xref<T>::template type, ca::impl::xref<U>::template type>::type;
		};

		template <typename T, typename U>
		concept is_cond_res_valid = requires {
			typename ca::impl::cond_res<T, U>;
		};

		template <typename T, typename U>
		concept is_common_type_valid = requires {
			typename ca::common_type<T, U>;
		};

		template <typename... Ts>
		struct common_reference final {
				using self = ca::impl::common_reference<Ts...>;
				common_reference(void) = delete;
		}; // struct common_reference

		template <typename T>
		struct common_reference<T> final {
				using self = ca::impl::common_reference<T>;
				using type = T;
				common_reference(void) = delete;
		}; // struct common_reference

		template <typename T, typename U>
		requires ca::impl::is_common_ref_valid<T, U>
		struct common_reference<T, U> final {
				using self = ca::impl::common_reference<T, U>;
				using type = typename ca::impl::common_ref<T, U>::type;
				common_reference(void) = delete;
		}; // struct common_reference

		template <typename T, typename U>
		requires (!ca::impl::is_common_ref_valid<T, U>) && ca::impl::is_basic_common_ref_valid<T, U>
		struct common_reference<T, U> final {
				using self = ca::impl::common_reference<T, U>;
				using type = typename ca::basic_common_reference<ca::remove_cvref<T>, ca::remove_cvref<U>,
						ca::impl::xref<T>::template type, ca::impl::xref<U>::template type>::type;
				common_reference(void) = delete;
		}; // struct common_reference

		template <typename T, typename U>
		requires (!ca::impl::is_common_ref_valid<T, U>) && (!ca::impl::is_basic_common_ref_valid<T, U>)
			&& ca::impl::is_cond_res_valid<T, U>
		struct common_reference<T, U> final {
				using self = ca::impl::common_reference<T, U>;
				using type = ca::impl::cond_res<T, U>;
				common_reference(void) = delete;
		}; // struct common_reference

		template <typename T, typename U>
		requires (!ca::impl::is_common_ref_valid<T, U>) && (!ca::impl::is_basic_common_ref_valid<T, U>)
			&& (!ca::impl::is_cond_res_valid<T, U>) && ca::impl::is_common_type_valid<T, U>
		struct common_reference<T, U> final {
				using self = ca::impl::common_reference<T, U>;
				using type = ca::common_type<T, U>;
				common_reference(void) = delete;
		}; // struct common_reference

		template <typename T, typename U, typename V, typename... Ws>
		requires requires {
			typename ca::impl::common_reference<typename ca::impl::common_reference<T, U>::type, V, Ws...>::type;
		}
		struct common_reference<T, U, V, Ws...> final {
				using self = ca::impl::common_reference<T, U, V, Ws...>;
				using type = typename ca::impl::common_reference<typename ca::impl::common_reference<T, U>::type, V, Ws...>::type;
				common_reference(void) = delete;
		}; // struct common_reference

	} // namespace impl


	/* @DOC std::common_reference_t. */
	template <typename... Ts>
	using common_reference = typename ca::impl::common_reference<Ts...>::type;

} // namespace ca

#endif // ca_primitives_traits_common_reference_hpp
