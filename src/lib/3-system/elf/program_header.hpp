#ifndef ca_system_elf_program_header_hpp
#define ca_system_elf_program_header_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- SEGMENT TYPES --------------------------------------------------

		/* @DOC p_type: what a segment is (PT_*). */
		namespace segment_types {

			inline constexpr ca::u32 null            = 0U;
			inline constexpr ca::u32 load            = 1U;
			inline constexpr ca::u32 dynamic         = 2U;
			inline constexpr ca::u32 interpreter     = 3U;
			inline constexpr ca::u32 note            = 4U;
			inline constexpr ca::u32 program_headers = 6U;
			inline constexpr ca::u32 tls             = 7U;
			inline constexpr ca::u32 gnu_eh_frame    = 0x6474E550U;
			inline constexpr ca::u32 gnu_stack       = 0x6474E551U;
			inline constexpr ca::u32 gnu_relro       = 0x6474E552U;
			inline constexpr ca::u32 gnu_property    = 0x6474E553U;

		} // namespace segment_types


		// -- SEGMENT FLAGS --------------------------------------------------

		/* @DOC p_flags: the access a loaded segment gets (PF_*). */
		namespace segment_flags {

			inline constexpr ca::u32 execute = 0x1U;
			inline constexpr ca::u32 write   = 0x2U;
			inline constexpr ca::u32 read    = 0x4U;

		} // namespace segment_flags


		// -- PROGRAM HEADER -------------------------------------------------

		/* @DOC An ELF64 program header (Elf64_Phdr): one segment, where it is in the file and
		   where it goes in memory (a link-time address; add the image's bias). */
		struct program_header final {

				using self = ca::elf::program_header;

				ca::u32 _type;
				ca::u32 _flags;
				ca::u64 _offset;
				ca::u64 _address;
				ca::u64 _physical;
				ca::u64 _file_size;
				ca::u64 _memory_size;
				ca::u64 _align;

		}; // struct program_header

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_program_header_hpp
