#ifndef ca_system_syscall_invoke_hpp
#define ca_system_syscall_invoke_hpp

#include "1-primitives/integers/isz.hpp"
#include "1-primitives/traits/is_enum.hpp"
#include "1-primitives/traits/is_null_pointer.hpp"
#include "1-primitives/traits/is_pointer.hpp"
#include "1-primitives/traits/underlying_type.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/syscall/argument.hpp"
#include "3-system/syscall/impl/enter.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYSCALL ------------------------------------------------------------

	/* @DOC Raw kernel entry points: invoke, plus one function per syscall of the target (number::*
	   and a call of the same name) in a table generated on that machine by
	   tooling/gen_syscalls.py. No checking of argument count or types beyond is_argument; prefer
	   the typed ca::sys wrappers. */
	namespace syscall {


		// -- INVOKE ---------------------------------------------------------

		namespace impl {

			/* @DOC An argument as a register value. */
			template <typename A>
			requires ca::syscall::is_argument<A>
			[[nodiscard]] inline auto word(const A argument) noexcept -> ca::isz {

				if constexpr (ca::is_null_pointer<A>)
					return 0;
				else if constexpr (ca::is_pointer<A>)
					return reinterpret_cast<ca::isz>(argument);
				else if constexpr (ca::is_enum<A>)
					return static_cast<ca::isz>(static_cast<ca::underlying_type<A>>(argument));
				else
					return static_cast<ca::isz>(argument);
			}

		} // namespace impl


		/* @DOC Makes syscall number with exactly these arguments, in as many registers; returns
		   what the kernel returns, or its errno as the error, also stored for sys::last_error.
		   Inline asm on linux x86_64, aarch64 and riscv64 (impl::inline_entry), libc's
		   syscall(2) on other linux CPUs, errc::not_supported on macOS. */
		template <typename... As>
		requires ca::syscall::are_arguments<As...>
		[[nodiscard]] inline auto invoke(const ca::isz number, const As... arguments) noexcept -> ca::result<ca::isz> {

			const ca::isz raw = ca::syscall::impl::enter(number, ca::syscall::impl::word(arguments)...);
			if (raw < 0 && raw > -4096) [[unlikely]] {
				const ca::error error{static_cast<int>(-raw)};
				ca::sys::set_last_error(error);
				return ca::failure{error};
			}
			return raw;
		}

	} // namespace syscall

} // namespace ca

#endif // ca_system_syscall_invoke_hpp
