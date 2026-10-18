#ifndef ca_system_memory_free_hpp
#define ca_system_memory_free_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FREE -----------------------------------------------------------

		/* @DOC Releases a block from mallocarray or reallocarray; nullptr does nothing. */
		auto free(void* block) noexcept -> void;

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_free_hpp
