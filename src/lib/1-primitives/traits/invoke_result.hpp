#ifndef ca_primitives_traits_invoke_result_hpp
#define ca_primitives_traits_invoke_result_hpp

#include "1-primitives/traits/is_base_of.hpp"
#include "1-primitives/traits/is_function.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/traits/unwrap_reference.hpp"
#include "1-primitives/utility/declval.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- INVOKE RESULT ------------------------------------------------------

	namespace impl {

		template <typename C, typename T>
		concept is_invoke_on_object = ca::is_base_of<C, ca::remove_cvref<T>>;

		template <typename T>
		concept is_invoke_on_wrapper = !ca::is_same<ca::unwrap_reference<ca::remove_cvref<T>>, ca::remove_cvref<T>>;

		template <typename F, typename... Args>
		auto invoke(F&& f, Args&&... args)
			noexcept(noexcept(static_cast<F&&>(f)(static_cast<Args&&>(args)...)))
			-> decltype(static_cast<F&&>(f)(static_cast<Args&&>(args)...));

		template <typename M, typename C, typename T, typename... Args>
		requires ca::is_function<M> && ca::impl::is_invoke_on_object<C, T>
		auto invoke(M C::* f, T&& t, Args&&... args)
			noexcept(noexcept((static_cast<T&&>(t).*f)(static_cast<Args&&>(args)...)))
			-> decltype((static_cast<T&&>(t).*f)(static_cast<Args&&>(args)...));

		template <typename M, typename C, typename T, typename... Args>
		requires ca::is_function<M> && (!ca::impl::is_invoke_on_object<C, T>) && ca::impl::is_invoke_on_wrapper<T>
		auto invoke(M C::* f, T&& t, Args&&... args)
			noexcept(noexcept((static_cast<T&&>(t).get().*f)(static_cast<Args&&>(args)...)))
			-> decltype((static_cast<T&&>(t).get().*f)(static_cast<Args&&>(args)...));

		template <typename M, typename C, typename T, typename... Args>
		requires ca::is_function<M> && (!ca::impl::is_invoke_on_object<C, T>) && (!ca::impl::is_invoke_on_wrapper<T>)
		auto invoke(M C::* f, T&& t, Args&&... args)
			noexcept(noexcept(((*static_cast<T&&>(t)).*f)(static_cast<Args&&>(args)...)))
			-> decltype(((*static_cast<T&&>(t)).*f)(static_cast<Args&&>(args)...));

		template <typename M, typename C, typename T>
		requires (!ca::is_function<M>) && ca::impl::is_invoke_on_object<C, T>
		auto invoke(M C::* f, T&& t)
			noexcept(noexcept(static_cast<T&&>(t).*f))
			-> decltype(static_cast<T&&>(t).*f);

		template <typename M, typename C, typename T>
		requires (!ca::is_function<M>) && (!ca::impl::is_invoke_on_object<C, T>) && ca::impl::is_invoke_on_wrapper<T>
		auto invoke(M C::* f, T&& t)
			noexcept(noexcept(static_cast<T&&>(t).get().*f))
			-> decltype(static_cast<T&&>(t).get().*f);

		template <typename M, typename C, typename T>
		requires (!ca::is_function<M>) && (!ca::impl::is_invoke_on_object<C, T>) && (!ca::impl::is_invoke_on_wrapper<T>)
		auto invoke(M C::* f, T&& t)
			noexcept(noexcept((*static_cast<T&&>(t)).*f))
			-> decltype((*static_cast<T&&>(t)).*f);

	} // namespace impl


	/* @DOC std::invoke_result_t. INVOKE rules. Reference wrappers are detected through
	   ca::impl::unwrap_reference and unwrapped with .get(). */
	template <typename F, typename... Args>
	using invoke_result = decltype(ca::impl::invoke(ca::declval<F>(), ca::declval<Args>()...));

} // namespace ca

#endif // ca_primitives_traits_invoke_result_hpp
