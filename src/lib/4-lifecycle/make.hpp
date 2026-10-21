#ifndef ca_lifecycle_make_hpp
#define ca_lifecycle_make_hpp

#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAKE ---------------------------------------------------------------

	namespace impl {

		/* @DOC T has an acquisition step for args: a static T::make(args...) -> result<T>. */
		template <typename T, typename... Args>
		concept has_make = requires(Args&&... args) {
			{ T::make(ca::forward<Args>(args)...) } -> ca::is_same<ca::result<T>>;
		};

		/* @DOC T has a constructor taking an error& first, through which it reports failure. */
		template <typename T, typename... Args>
		concept has_reporting_constructor = ca::is_constructible<T, ca::error&, Args...>;

		template <typename T, typename... Args>
		concept is_makeable = ca::impl::has_make<T, Args...> || ca::impl::has_reporting_constructor<T, Args...> || ca::is_constructible<T, Args...>;

		/* @DOC Builds a T from args in memory for one, as ca::make builds it. On failure no T is
		   left there. */
		template <typename T, typename... Args>
		requires ca::impl::is_makeable<T, Args...>
		[[nodiscard]] constexpr auto make_at(T* const at, Args&&... args) noexcept -> ca::result<void> {

			if constexpr (ca::impl::has_make<T, Args...>) {
				auto made = T::make(ca::forward<Args>(args)...);
				if (!made) [[unlikely]]
					return made.propagate();
				ca::construct_at(at, ca::move(*made));
			}
			else if constexpr (ca::impl::has_reporting_constructor<T, Args...>) {
				ca::error error{};
				ca::construct_at(at, error, ca::forward<Args>(args)...);
				if (!error.ok()) [[unlikely]] {
					ca::destroy_at(at);
					return ca::failure{error};
				}
			}
			else
				ca::construct_at(at, ca::forward<Args>(args)...);
			return ca::result<void>{};
		}

	} // namespace impl


	/* @DOC Builds a T from args, every failure in the result: through T::make(args...) when T
	   has it (its acquisition step: what can fail happens there, then a constructor that cannot
	   fail), else T(error&, args...) when T has it (the constructor reports through error), else
	   T(args...). A failure leaves no T. T::make builds through T's constructors, never through
	   ca::make<T>. */
	template <typename T, typename... Args>
	requires ca::impl::is_makeable<T, Args...>
	[[nodiscard]] constexpr auto make(Args&&... args) noexcept -> ca::result<T> {

		if constexpr (ca::impl::has_make<T, Args...>)
			return T::make(ca::forward<Args>(args)...);
		else if constexpr (ca::impl::has_reporting_constructor<T, Args...>) {
			ca::error error{};
			ca::result<T> made{ca::in_place, error, ca::forward<Args>(args)...};
			if (!error.ok()) [[unlikely]]
				return ca::failure{error};
			return made;
		}
		else
			return ca::result<T>{ca::in_place, ca::forward<Args>(args)...};
	}

} // namespace ca

#endif // ca_lifecycle_make_hpp
