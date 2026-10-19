#ifndef ca_system_file_impl_temporary_name_hpp
#define ca_system_file_impl_temporary_name_hpp

#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TEMPORARY NAME -------------------------------------------------

		namespace impl {

			/* @DOC How many names mkdtemp and mkstemp try before giving up with errc::exists. */
			inline constexpr int TEMPORARY_ATTEMPTS = 100;

			/* @DOC The "XXXXXX" ending pattern, which must have one (errc::invalid_argument
			   otherwise, as set_last_error). */
			[[nodiscard]] auto temporary_suffix(char* pattern) noexcept -> ca::result<char*>;

			/* @DOC Writes 6 random characters of [A-Za-z0-9] at suffix (kernel entropy). */
			[[nodiscard]] auto randomize_suffix(char* suffix) noexcept -> ca::result<void>;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_file_impl_temporary_name_hpp
