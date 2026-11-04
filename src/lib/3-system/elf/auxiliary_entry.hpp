#ifndef ca_system_elf_auxiliary_entry_hpp
#define ca_system_elf_auxiliary_entry_hpp

#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- AUXILIARY TYPES ------------------------------------------------

		/* @DOC Types of the auxiliary vector's entries (AT_*), what the kernel tells a process it
		   starts: its program headers, page size, interpreter base, entry point, ids, 16 random
		   bytes, the vDSO's ELF header. */
		namespace auxiliary_types {

			inline constexpr ca::u64 null                    = 0U;
			inline constexpr ca::u64 executable_fd           = 2U;
			inline constexpr ca::u64 program_headers         = 3U;
			inline constexpr ca::u64 program_header_size     = 4U;
			inline constexpr ca::u64 program_header_count    = 5U;
			inline constexpr ca::u64 page_size               = 6U;
			inline constexpr ca::u64 interpreter_base        = 7U;
			inline constexpr ca::u64 flags                   = 8U;
			inline constexpr ca::u64 entry                   = 9U;
			inline constexpr ca::u64 uid                     = 11U;
			inline constexpr ca::u64 effective_uid           = 12U;
			inline constexpr ca::u64 gid                     = 13U;
			inline constexpr ca::u64 effective_gid           = 14U;
			inline constexpr ca::u64 platform                = 15U;
			inline constexpr ca::u64 hardware_capabilities   = 16U;
			inline constexpr ca::u64 clock_ticks             = 17U;
			inline constexpr ca::u64 secure                  = 23U;
			inline constexpr ca::u64 random                  = 25U;
			inline constexpr ca::u64 hardware_capabilities_2 = 26U;
			inline constexpr ca::u64 executable_name         = 31U;
			inline constexpr ca::u64 vdso                    = 33U;
			inline constexpr ca::u64 minimum_signal_stack    = 51U;

		} // namespace auxiliary_types


		// -- AUXILIARY ENTRY ------------------------------------------------

		/* @DOC An entry of the auxiliary vector (Elf64_auxv_t), past the environment on the
		   initial stack; the vector ends at the entry typed auxiliary_types::null. */
		struct auxiliary_entry final {

				using self = ca::elf::auxiliary_entry;

				ca::u64 _type;
				ca::u64 _value;

		}; // struct auxiliary_entry

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_auxiliary_entry_hpp
