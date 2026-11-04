#ifndef ca_system_elf_relocation_hpp
#define ca_system_elf_relocation_hpp

#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- X86 64 RELOCATIONS ---------------------------------------------

		/* @DOC The x86-64 relocation types a dynamic loader applies (R_X86_64_*). S is the
		   symbol's address, A the addend, B the image's bias. absolute: S + A; global_data and
		   jump_slot: S; relative: B + A; indirect_relative: the result of calling B + A;
		   tls_module: the module id; tls_offset: S's offset in its module's block;
		   tls_thread_offset: S's offset from the thread pointer; tls_descriptor: a TLS
		   descriptor for S (-mtls-dialect=gnu2). copy copies S's bytes into the executable. */
		namespace x86_64_relocations {

			inline constexpr ca::u32 none              = 0U;
			inline constexpr ca::u32 absolute          = 1U;
			inline constexpr ca::u32 copy              = 5U;
			inline constexpr ca::u32 global_data       = 6U;
			inline constexpr ca::u32 jump_slot         = 7U;
			inline constexpr ca::u32 relative          = 8U;
			inline constexpr ca::u32 tls_module        = 16U;
			inline constexpr ca::u32 tls_offset        = 17U;
			inline constexpr ca::u32 tls_thread_offset = 18U;
			inline constexpr ca::u32 tls_descriptor    = 36U;
			inline constexpr ca::u32 indirect_relative = 37U;

		} // namespace x86_64_relocations


		// -- RELOCATION -----------------------------------------------------

		/* @DOC An ELF64 relocation with addend (Elf64_Rela): the link-time address to patch, the
		   symbol index and type packed in _info, the addend. */
		struct relocation final {

				using self = ca::elf::relocation;

				ca::u64 _offset;
				ca::u64 _info;
				ca::i64 _addend;

				/* @DOC Index of the symbol in the dynamic symbol table, 0 for none. */
				[[nodiscard]] constexpr auto symbol(void) const noexcept -> ca::u32 {
					return static_cast<ca::u32>(_info >> 32U);
				}

				/* @DOC The machine's relocation type (x86_64_relocations on x86-64). */
				[[nodiscard]] constexpr auto type(void) const noexcept -> ca::u32 {
					return static_cast<ca::u32>(_info);
				}

		}; // struct relocation

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_relocation_hpp
