#ifndef ca_system_thread_yield_hpp
#define ca_system_thread_yield_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- YIELD ----------------------------------------------------------

		/* @DOC Gives the CPU to another runnable thread, if any. */
		auto yield(void) noexcept -> void;

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_yield_hpp
