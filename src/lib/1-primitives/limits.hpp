#ifndef ca_primitives_limits_hpp
#define ca_primitives_limits_hpp

#include "1-primitives/traits/is_floating_point.hpp"
#include "1-primitives/traits/is_integral.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_signed.hpp"
#include "1-primitives/traits/make_unsigned.hpp"
#include "1-primitives/traits/remove_cv.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LIMITS -------------------------------------------------------------

	/* @DOC Range of an arithmetic type: min, max, lowest. Floating points add epsilon, infinity,
	   quiet_nan; their min is the smallest positive normal. Not instantiable. */
	template <typename T>
	struct limits final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::limits<T>;


		public:

			// -- public lifecycle -------------------------------------------

			limits(void) = delete;


			// -- accessors --------------------------------------------------

			static consteval auto min(void) noexcept -> T requires ca::is_integral<T> {

				if constexpr (ca::is_signed<T>)
					return static_cast<T>(-self::max() - 1);
				else
					return static_cast<T>(0U);
			}

			static consteval auto max(void) noexcept -> T requires ca::is_integral<T> {

				if constexpr (ca::is_same<ca::remove_cv<T>, bool>)
					return true;
				else if constexpr (ca::is_signed<T>)
					return static_cast<T>(static_cast<ca::make_unsigned<T>>(~static_cast<ca::make_unsigned<T>>(0U)) >> 1U);
				else
					return static_cast<T>(~static_cast<T>(0U));
			}

			static consteval auto lowest(void) noexcept -> T requires ca::is_integral<T> {
				return self::min();
			}

			static consteval auto min(void) noexcept -> T requires ca::is_floating_point<T> {
				return self::_pick(__FLT_MIN__, __DBL_MIN__, __LDBL_MIN__);
			}

			static consteval auto max(void) noexcept -> T requires ca::is_floating_point<T> {
				return self::_pick(__FLT_MAX__, __DBL_MAX__, __LDBL_MAX__);
			}

			static consteval auto lowest(void) noexcept -> T requires ca::is_floating_point<T> {
				return -self::max();
			}

			static consteval auto epsilon(void) noexcept -> T requires ca::is_floating_point<T> {
				return self::_pick(__FLT_EPSILON__, __DBL_EPSILON__, __LDBL_EPSILON__);
			}

			static consteval auto infinity(void) noexcept -> T requires ca::is_floating_point<T> {
				return self::_pick(__builtin_huge_valf(), __builtin_huge_val(), __builtin_huge_vall());
			}

			static consteval auto quiet_nan(void) noexcept -> T requires ca::is_floating_point<T> {
				return self::_pick(__builtin_nanf(""), __builtin_nan(""), __builtin_nanl(""));
			}


		private:

			// -- private methods --------------------------------------------

			static consteval auto _pick(const float f, const double d, const long double l) noexcept -> T {

				if constexpr (ca::is_same<ca::remove_cv<T>, float>)
					return f;
				else if constexpr (ca::is_same<ca::remove_cv<T>, double>)
					return d;
				else
					return l;
			}

	}; // struct limits

} // namespace ca

#endif // ca_primitives_limits_hpp
