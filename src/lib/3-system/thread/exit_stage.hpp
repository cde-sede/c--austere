#ifndef ca_system_thread_exit_stage_hpp
#define ca_system_thread_exit_stage_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- EXIT STAGE -----------------------------------------------------

		/* @DOC When a thread key's exit callback runs, among those of the exiting thread: every
		   user one, then every buffers one (output flushed: it may still allocate), then every
		   memory one (the thread's heap handed back). Within a stage, the newest key first. */
		enum class exit_stage {
			user,
			buffers,
			memory,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_exit_stage_hpp
