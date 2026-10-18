#ifndef ca_system_file_seek_origin_hpp
#define ca_system_file_seek_origin_hpp




// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SEEK ORIGIN ----------------------------------------------------

		/* @DOC Where a seek offset counts from. */
		enum class seek_origin {
			start,
			current,
			end,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_file_seek_origin_hpp
