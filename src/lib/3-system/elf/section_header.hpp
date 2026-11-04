#ifndef ca_system_elf_section_header_hpp
#define ca_system_elf_section_header_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- SECTION TYPES --------------------------------------------------

		/* @DOC sh_type: what a section holds (SHT_*). */
		namespace section_types {

			inline constexpr ca::u32 null            = 0U;
			inline constexpr ca::u32 program_bits    = 1U;
			inline constexpr ca::u32 symbols         = 2U;
			inline constexpr ca::u32 strings         = 3U;
			inline constexpr ca::u32 rela            = 4U;
			inline constexpr ca::u32 hash            = 5U;
			inline constexpr ca::u32 dynamic         = 6U;
			inline constexpr ca::u32 note            = 7U;
			inline constexpr ca::u32 no_bits         = 8U;
			inline constexpr ca::u32 rel             = 9U;
			inline constexpr ca::u32 dynamic_symbols = 11U;
			inline constexpr ca::u32 init_array      = 14U;
			inline constexpr ca::u32 fini_array      = 15U;
			inline constexpr ca::u32 gnu_hash        = 0x6FFFFFF6U;

		} // namespace section_types


		// -- SECTION FLAGS --------------------------------------------------

		/* @DOC sh_flags (SHF_*). */
		namespace section_flags {

			inline constexpr ca::u64 write      = 0x1U;
			inline constexpr ca::u64 alloc      = 0x2U;
			inline constexpr ca::u64 execute    = 0x4U;
			inline constexpr ca::u64 tls        = 0x400U;
			inline constexpr ca::u64 compressed = 0x800U;

		} // namespace section_flags


		// -- SECTION INDICES ------------------------------------------------

		/* @DOC Reserved section indices a symbol's st_shndx may hold (SHN_*). */
		namespace section_indices {

			inline constexpr ca::u16 undefined = 0U;
			inline constexpr ca::u16 absolute  = 0xFFF1U;
			inline constexpr ca::u16 common    = 0xFFF2U;

		} // namespace section_indices


		// -- SECTION HEADER -------------------------------------------------

		/* @DOC An ELF64 section header (Elf64_Shdr). Its name is an offset into the section
		   names table (file_header::_section_names); elf::file reads it. */
		struct section_header final {

				using self = ca::elf::section_header;

				ca::u32 _name;
				ca::u32 _type;
				ca::u64 _flags;
				ca::u64 _address;
				ca::u64 _offset;
				ca::u64 _size;
				ca::u32 _link;
				ca::u32 _info;
				ca::u64 _align;
				ca::u64 _entry_size;

		}; // struct section_header

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_section_header_hpp
