#ifndef ca_system_process_mask_change_hpp
#define ca_system_process_mask_change_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- MASK CHANGE ----------------------------------------------------

		/* @DOC How sigprocmask changes the blocked signals: adds the set, removes it, or becomes it. */
		enum class mask_change {
			block,
			unblock,
			replace,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_process_mask_change_hpp
