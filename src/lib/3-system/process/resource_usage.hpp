#ifndef ca_system_process_resource_usage_hpp
#define ca_system_process_resource_usage_hpp

#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- RESOURCE USAGE -------------------------------------------------

		/* @DOC What getrusage reports: CPU time, peak memory, page faults, context switches. */
		class resource_usage final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::resource_usage;


				// -- private members ----------------------------------------

				ca::i64 _user_time;
				ca::i64 _system_time;
				ca::u64 _max_resident;
				ca::u64 _minor_faults;
				ca::u64 _major_faults;
				ca::u64 _voluntary_switches;
				ca::u64 _involuntary_switches;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr resource_usage(const ca::i64 user_time, const ca::i64 system_time, const ca::u64 max_resident, const ca::u64 minor_faults, const ca::u64 major_faults, const ca::u64 voluntary_switches, const ca::u64 involuntary_switches) noexcept
				: _user_time{user_time}, _system_time{system_time}, _max_resident{max_resident}, _minor_faults{minor_faults}, _major_faults{major_faults}, _voluntary_switches{voluntary_switches}, _involuntary_switches{involuntary_switches} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC CPU time in user mode, nanoseconds. */
				[[nodiscard]] constexpr auto user_time(void) const noexcept -> ca::i64 {
					return _user_time;
				}

				/* @DOC CPU time in the kernel, nanoseconds. */
				[[nodiscard]] constexpr auto system_time(void) const noexcept -> ca::i64 {
					return _system_time;
				}

				/* @DOC Peak resident memory, bytes. */
				[[nodiscard]] constexpr auto max_resident(void) const noexcept -> ca::u64 {
					return _max_resident;
				}

				/* @DOC Page faults served without I/O. */
				[[nodiscard]] constexpr auto minor_faults(void) const noexcept -> ca::u64 {
					return _minor_faults;
				}

				/* @DOC Page faults that needed I/O. */
				[[nodiscard]] constexpr auto major_faults(void) const noexcept -> ca::u64 {
					return _major_faults;
				}

				/* @DOC Times the process gave up the CPU to wait. */
				[[nodiscard]] constexpr auto voluntary_switches(void) const noexcept -> ca::u64 {
					return _voluntary_switches;
				}

				/* @DOC Times the scheduler took the CPU away. */
				[[nodiscard]] constexpr auto involuntary_switches(void) const noexcept -> ca::u64 {
					return _involuntary_switches;
				}

		}; // class resource_usage

	} // namespace sys

} // namespace ca

#endif // ca_system_process_resource_usage_hpp
