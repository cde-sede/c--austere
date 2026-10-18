#ifndef ca_system_memory_remapping_hpp
#define ca_system_memory_remapping_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- REMAPPING ------------------------------------------------------

		/* @DOC Whether mremap must keep the mapping at its address (in_place) or may move it. */
		enum class remapping {
			in_place,
			may_move,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_memory_remapping_hpp
