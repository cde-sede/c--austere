#ifndef ca_system_memory_sharing_hpp
#define ca_system_memory_sharing_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SHARING --------------------------------------------------------

		/* @DOC Whether writes to a file mapping reach the file and other mappings (shared), or stay in
		   private copies of the pages written (copy_on_write). */
		enum class sharing {
			shared,
			copy_on_write,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_sharing_hpp
