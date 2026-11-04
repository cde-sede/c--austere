#ifndef ca_system_elf_dynamic_entry_hpp
#define ca_system_elf_dynamic_entry_hpp

#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- DYNAMIC TAGS ---------------------------------------------------

		/* @DOC d_tag: what an entry of the dynamic section gives (DT_*). Addresses are link-time
		   addresses, sizes in bytes. */
		namespace dynamic_tags {

			inline constexpr ca::i64 null                  = 0;
			inline constexpr ca::i64 needed                = 1;
			inline constexpr ca::i64 plt_relocations_size  = 2;
			inline constexpr ca::i64 plt_got               = 3;
			inline constexpr ca::i64 hash                  = 4;
			inline constexpr ca::i64 strings               = 5;
			inline constexpr ca::i64 symbols               = 6;
			inline constexpr ca::i64 rela                  = 7;
			inline constexpr ca::i64 rela_size             = 8;
			inline constexpr ca::i64 rela_entry            = 9;
			inline constexpr ca::i64 strings_size          = 10;
			inline constexpr ca::i64 symbol_entry          = 11;
			inline constexpr ca::i64 init                  = 12;
			inline constexpr ca::i64 fini                  = 13;
			inline constexpr ca::i64 soname                = 14;
			inline constexpr ca::i64 rpath                 = 15;
			inline constexpr ca::i64 symbolic              = 16;
			inline constexpr ca::i64 rel                   = 17;
			inline constexpr ca::i64 rel_size              = 18;
			inline constexpr ca::i64 rel_entry             = 19;
			inline constexpr ca::i64 plt_relocation_kind   = 20;
			inline constexpr ca::i64 debug                 = 21;
			inline constexpr ca::i64 text_relocations      = 22;
			inline constexpr ca::i64 plt_relocations       = 23;
			inline constexpr ca::i64 bind_now              = 24;
			inline constexpr ca::i64 init_array            = 25;
			inline constexpr ca::i64 fini_array            = 26;
			inline constexpr ca::i64 init_array_size       = 27;
			inline constexpr ca::i64 fini_array_size       = 28;
			inline constexpr ca::i64 runpath               = 29;
			inline constexpr ca::i64 flags                 = 30;
			inline constexpr ca::i64 preinit_array         = 32;
			inline constexpr ca::i64 preinit_array_size    = 33;
			inline constexpr ca::i64 relr_size             = 35;
			inline constexpr ca::i64 relr                  = 36;
			inline constexpr ca::i64 relr_entry            = 37;
			inline constexpr ca::i64 gnu_hash              = 0x6FFFFEF5;
			inline constexpr ca::i64 version_symbols       = 0x6FFFFFF0;
			inline constexpr ca::i64 rela_count            = 0x6FFFFFF9;
			inline constexpr ca::i64 flags_1               = 0x6FFFFFFB;
			inline constexpr ca::i64 version_definitions   = 0x6FFFFFFC;
			inline constexpr ca::i64 version_definition_count = 0x6FFFFFFD;
			inline constexpr ca::i64 version_needs         = 0x6FFFFFFE;
			inline constexpr ca::i64 version_need_count    = 0x6FFFFFFF;

		} // namespace dynamic_tags


		// -- DYNAMIC FLAGS --------------------------------------------------

		/* @DOC Bits of the dynamic_tags::flags entry (DF_*). */
		namespace dynamic_flags {

			inline constexpr ca::u64 origin           = 0x1U;
			inline constexpr ca::u64 symbolic         = 0x2U;
			inline constexpr ca::u64 text_relocations = 0x4U;
			inline constexpr ca::u64 bind_now         = 0x8U;
			inline constexpr ca::u64 static_tls       = 0x10U;

		} // namespace dynamic_flags


		// -- DYNAMIC FLAGS 1 ------------------------------------------------

		/* @DOC Bits of the dynamic_tags::flags_1 entry (DF_1_*). */
		namespace dynamic_flags_1 {

			inline constexpr ca::u64 now       = 0x1U;
			inline constexpr ca::u64 global    = 0x2U;
			inline constexpr ca::u64 no_delete = 0x8U;
			inline constexpr ca::u64 pie       = 0x08000000U;

		} // namespace dynamic_flags_1


		// -- DYNAMIC ENTRY --------------------------------------------------

		/* @DOC An entry of the dynamic section (Elf64_Dyn): a tag and its value, an address or a
		   number; the section ends at the entry tagged dynamic_tags::null. */
		struct dynamic_entry final {

				using self = ca::elf::dynamic_entry;

				ca::i64 _tag;
				ca::u64 _value;

		}; // struct dynamic_entry

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_dynamic_entry_hpp
