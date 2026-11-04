#ifndef ca_system_elf_symbol_hpp
#define ca_system_elf_symbol_hpp

#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "3-system/elf/section_header.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- SYMBOL TYPES ---------------------------------------------------

		/* @DOC What a symbol names (STT_*). */
		namespace symbol_types {

			inline constexpr ca::u8 none      = 0U;
			inline constexpr ca::u8 object    = 1U;
			inline constexpr ca::u8 function  = 2U;
			inline constexpr ca::u8 section   = 3U;
			inline constexpr ca::u8 file      = 4U;
			inline constexpr ca::u8 common    = 5U;
			inline constexpr ca::u8 tls       = 6U;
			inline constexpr ca::u8 gnu_ifunc = 10U;

		} // namespace symbol_types


		// -- SYMBOL BINDINGS ------------------------------------------------

		/* @DOC Who sees a symbol (STB_*). */
		namespace symbol_bindings {

			inline constexpr ca::u8 local      = 0U;
			inline constexpr ca::u8 global     = 1U;
			inline constexpr ca::u8 weak       = 2U;
			inline constexpr ca::u8 gnu_unique = 10U;

		} // namespace symbol_bindings


		// -- SYMBOL VISIBILITIES --------------------------------------------

		/* @DOC Visibility (STV_*): normal is STV_DEFAULT, nonpreemptible STV_PROTECTED. */
		namespace symbol_visibilities {

			inline constexpr ca::u8 normal         = 0U;
			inline constexpr ca::u8 internal       = 1U;
			inline constexpr ca::u8 hidden         = 2U;
			inline constexpr ca::u8 nonpreemptible = 3U;

		} // namespace symbol_visibilities


		// -- SYMBOL ---------------------------------------------------------

		/* @DOC An ELF64 symbol (Elf64_Sym). Its name is an offset into the string table linked
		   to its symbol table; its value a link-time address for a defined symbol. */
		struct symbol final {

				using self = ca::elf::symbol;

				ca::u32 _name;
				ca::u8 _info;
				ca::u8 _other;
				ca::u16 _section;
				ca::u64 _value;
				ca::u64 _size;

				/* @DOC One of elf::symbol_types. */
				[[nodiscard]] constexpr auto type(void) const noexcept -> ca::u8 {
					return static_cast<ca::u8>(_info & 0xFU);
				}

				/* @DOC One of elf::symbol_bindings. */
				[[nodiscard]] constexpr auto binding(void) const noexcept -> ca::u8 {
					return static_cast<ca::u8>(_info >> 4U);
				}

				/* @DOC One of elf::symbol_visibilities. */
				[[nodiscard]] constexpr auto visibility(void) const noexcept -> ca::u8 {
					return static_cast<ca::u8>(_other & 0x3U);
				}

				/* @DOC Whether the object holding the table defines it. */
				[[nodiscard]] constexpr auto is_defined(void) const noexcept -> bool {
					return _section != ca::elf::section_indices::undefined;
				}

		}; // struct symbol

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_symbol_hpp
