#ifndef ca_system_syscall_argument_hpp
#define ca_system_syscall_argument_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_integral.hpp"
#include "1-primitives/traits/is_null_pointer.hpp"
#include "1-primitives/traits/is_pointer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYSCALL ------------------------------------------------------------

	namespace syscall {


		// -- ARGUMENT -------------------------------------------------------

		/* @DOC What a raw syscall takes in a register: an integer, an enum or a pointer. */
		template <typename A>
		concept is_argument = ca::is_integral<A> || ca::is_enum<A> || ca::is_pointer<A> || ca::is_null_pointer<A>;

		/* @DOC At most six syscall arguments, the kernel's limit. */
		template <typename... As>
		concept are_arguments = sizeof...(As) <= ca::usz{6U} && (ca::syscall::is_argument<As> && ...);

	} // namespace syscall

} // namespace ca

#endif // ca_system_syscall_argument_hpp
