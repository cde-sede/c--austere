#ifndef ca_system_thread_thread_key_hpp
#define ca_system_thread_thread_key_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- THREAD KEY -----------------------------------------------------

		/* @DOC A per-thread slot from thread_key_create, holding one pointer per thread. Never
		   deleted. */
		class thread_key final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::thread_key;


				// -- private members ----------------------------------------

				ca::usz _id;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr explicit thread_key(const ca::usz id) noexcept
				: _id{id} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC The system's key, widened. */
				[[nodiscard]] constexpr auto id(void) const noexcept -> ca::usz {
					return _id;
				}

		}; // class thread_key

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_thread_key_hpp
