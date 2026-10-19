#ifndef ca_system_syscall_vdso_hpp
#define ca_system_syscall_vdso_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYSCALL ------------------------------------------------------------

	namespace syscall {


		// -- VDSO -----------------------------------------------------------

		/* @DOC Address of function name in the vDSO, the shared object the kernel maps into every
		   process (clock_gettime, gettimeofday, time, getcpu without entering the kernel), or null:
		   no vDSO, or no such symbol. Walks its ELF dynamic symbols at each call: look up once
		   and keep the address. Linux x86-64; null elsewhere. */
		[[nodiscard]] auto vdso_symbol(const char* name) noexcept -> void*;

	} // namespace syscall

} // namespace ca

#endif // ca_system_syscall_vdso_hpp
