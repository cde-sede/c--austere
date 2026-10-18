#ifndef ca_lifecycle_make_hpp
#define ca_lifecycle_make_hpp

#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAKE ---------------------------------------------------------------

	namespace impl {

		/* @DOC T has a constructor taking an error& first, through which it reports failure. */
		template <typename T, typename... Args>
		concept has_reporting_constructor = ca::is_constructible<T, ca::error&, Args...>;

		template <typename T, typename... Args>
		concept is_makeable = ca::impl::has_reporting_constructor<T, Args...> || ca::is_constructible<T, Args...>;

	} // namespace impl


	/* @DOC Builds a T from args: T(error&, args...) when T has it, else T(args...). A constructor
	   setting the error fails the make: the object is destroyed (its destructor runs as a
	   failsafe) and the error returned. Constructors initialize only; resources come from
	   fallible methods or factories. */
	template <typename T, typename... Args>
	requires ca::impl::is_makeable<T, Args...>
	[[nodiscard]] constexpr auto make(Args&&... args) noexcept -> ca::result<T> {

		if constexpr (ca::impl::has_reporting_constructor<T, Args...>) {
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
