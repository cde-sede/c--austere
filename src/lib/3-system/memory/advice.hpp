#ifndef ca_system_memory_advice_hpp
#define ca_system_memory_advice_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- ADVICE ---------------------------------------------------------

		/* @DOC How a mapping will be used, for madvise. dont_need drops the pages: on Linux, private ones
		   read back as zeros or from the file; macOS may keep them. */
		enum class advice {
			normal,
			random,
			sequential,
			will_need,
			dont_need,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_advice_hpp
