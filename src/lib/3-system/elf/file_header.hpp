#ifndef ca_system_elf_file_header_hpp
#define ca_system_elf_file_header_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- IDENTITY -------------------------------------------------------

		/* @DOC The identification bytes opening every ELF file (e_ident): indices into them, and
		   the values ca reads, ELF64 little-endian of the current version. */
		namespace identity {

			inline constexpr ca::usz size          = 16U;
			inline constexpr ca::usz class_index   = 4U;
			inline constexpr ca::usz data_index    = 5U;
			inline constexpr ca::usz version_index = 6U;
			inline constexpr ca::usz abi_index     = 7U;

			inline constexpr ca::u8 class_64      = 2U;
			inline constexpr ca::u8 little_endian = 1U;
			inline constexpr ca::u8 current       = 1U;

		} // namespace identity


		// -- FILE TYPES -----------------------------------------------------

		/* @DOC e_type: what the file is (ET_*). A position-independent executable is shared. */
		namespace file_types {

			inline constexpr ca::u16 none        = 0U;
			inline constexpr ca::u16 relocatable = 1U;
			inline constexpr ca::u16 executable  = 2U;
			inline constexpr ca::u16 shared      = 3U;
			inline constexpr ca::u16 core        = 4U;

		} // namespace file_types


		// -- MACHINES -------------------------------------------------------

		/* @DOC e_machine: the instruction set (EM_*). */
		namespace machines {

			inline constexpr ca::u16 x86_64  = 62U;
			inline constexpr ca::u16 aarch64 = 183U;
			inline constexpr ca::u16 riscv   = 243U;

		} // namespace machines


		// -- FILE HEADER ----------------------------------------------------

		/* @DOC The ELF64 file header (Elf64_Ehdr), at offset 0 of a file and at the load address
		   of an image mapping it. Same layout, checked against <elf.h> when the library builds. */
		struct file_header final {

				using self = ca::elf::file_header;

				ca::u8 _identity[ca::elf::identity::size];
				ca::u16 _type;
				ca::u16 _machine;
				ca::u32 _version;
				ca::u64 _entry;
				ca::u64 _segments_offset;
				ca::u64 _sections_offset;
				ca::u32 _flags;
				ca::u16 _header_size;
				ca::u16 _segment_size;
				ca::u16 _segment_count;
				ca::u16 _section_size;
				ca::u16 _section_count;
				ca::u16 _section_names;

				/* @DOC Whether the header opens an ELF64 little-endian file of the current
				   version whose header and table entries have the ELF64 sizes. */
				[[nodiscard]] constexpr auto is_elf64(void) const noexcept -> bool {
					return _identity[0] == 0x7FU && _identity[1] == 'E' && _identity[2] == 'L' && _identity[3] == 'F'
					    && _identity[ca::elf::identity::class_index] == ca::elf::identity::class_64
					    && _identity[ca::elf::identity::data_index] == ca::elf::identity::little_endian
					    && _identity[ca::elf::identity::version_index] == ca::elf::identity::current
					    && _header_size == sizeof(self)
					    && (_segment_count == 0U || _segment_size == 56U)
					    && (_section_count == 0U || _section_size == 64U);
				}

		}; // struct file_header

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_file_header_hpp
