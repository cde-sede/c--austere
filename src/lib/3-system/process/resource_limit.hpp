#ifndef ca_system_process_resource_limit_hpp
#define ca_system_process_resource_limit_hpp

#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RESOURCE LIMIT -------------------------------------------------

		/* @DOC Soft (current) and hard (maximum) bound of a resource. Only privileged processes raise
		   maximum; current stays at most maximum. */
		class resource_limit final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::resource_limit;


				// -- private members ----------------------------------------

				ca::u64 _current;
				ca::u64 _maximum;


			public:

				// -- public constants ---------------------------------------

				/* @DOC No bound. */
				static constexpr ca::u64 UNLIMITED = 0xFFFFFFFFFFFFFFFFU;


				// -- public lifecycle ---------------------------------------

				constexpr resource_limit(const ca::u64 current, const ca::u64 maximum) noexcept
				: _current{current}, _maximum{maximum} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC Bound enforced now, or UNLIMITED. */
				[[nodiscard]] constexpr auto current(void) const noexcept -> ca::u64 {
					return _current;
				}

				/* @DOC Ceiling for current, or UNLIMITED. */
				[[nodiscard]] constexpr auto maximum(void) const noexcept -> ca::u64 {
					return _maximum;
				}

		}; // class resource_limit

	} // namespace sys

} // namespace ca

#endif // ca_system_process_resource_limit_hpp
