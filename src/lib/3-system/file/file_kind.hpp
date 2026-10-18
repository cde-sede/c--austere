#ifndef ca_system_file_file_kind_hpp
#define ca_system_file_file_kind_hpp




// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FILE KIND ------------------------------------------------------

		/* @DOC Type of a file, from its mode. */
		enum class file_kind {
			regular,
			directory,
			symlink,
			fifo,
			socket,
			block_device,
			character_device,
			unknown,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_file_file_kind_hpp
