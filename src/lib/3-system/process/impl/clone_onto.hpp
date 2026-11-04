#ifndef ca_system_process_impl_clone_onto_hpp
#define ca_system_process_impl_clone_onto_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/isz.hpp"
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- CLONE ONTO -----------------------------------------------------

		namespace impl {

			/* @DOC clone with the child on a stack of its own: stack is its top, 16-aligned. The
			   child comes out of the syscall in assembly, with no C++ frame to return into, runs
			   entry(argument) and exits with the result, ending that task only. ptid, ctid and tls
			   are what flags makes of them (CLONE_PARENT_SETTID, CLONE_CHILD_SETTID or
			   CLONE_CHILD_CLEARTID, CLONE_SETTLS), null or 0 otherwise. Returns the child's id, in
			   the parent only. Linux x86-64 only. */
			[[nodiscard]] auto clone_onto(ca::isz flags, void* stack, ca::i32* ptid, ca::i32* ctid, ca::u64 tls, int (*entry)(void*), void* argument) noexcept -> ca::result<ca::i32>;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_process_impl_clone_onto_hpp
