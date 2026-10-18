#ifndef ca_system_time_clock_hpp
#define ca_system_time_clock_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CLOCK ----------------------------------------------------------

		/* @DOC Clocks to read with now(). */
		enum class clock {
			realtime,
			monotonic,
			process_cpu,
			thread_cpu,
		};

	} // namespace sys

} // namespace ca

#endif // ca_system_time_clock_hpp
