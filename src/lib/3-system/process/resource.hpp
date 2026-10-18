#ifndef ca_system_process_resource_hpp
#define ca_system_process_resource_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RESOURCE -------------------------------------------------------

		/* @DOC Resources getrlimit and setrlimit bound. */
		enum class resource {
			address_space,
			core_size,
			cpu_time,
			data_size,
			file_size,
			open_files,
			processes,
			stack_size,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_process_resource_hpp
