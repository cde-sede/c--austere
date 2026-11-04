#ifndef ca_system_elf_file_hpp
#define ca_system_elf_file_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/elf/file_header.hpp"
#include "3-system/elf/program_header.hpp"
#include "3-system/elf/section_header.hpp"
#include "3-system/elf/symbol.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- FILE -----------------------------------------------------------

		/* @DOC A view of the bytes of an ELF64 little-endian file, read as it is on disk: header,
		   program headers, sections. Made by from, which checks the header and that both tables
		   lie inside the bytes, aligned; every accessor then stays inside them, so a malformed
		   file gives empty results, never a fault. The bytes must start 8-aligned (a mapping
		   is). Owns nothing. */
		class file final {


			private:

				// -- private types ------------------------------------------

				using self = ca::elf::file;


				// -- private members ----------------------------------------

				ca::span<const ca::byte> _bytes;


				// -- private lifecycle --------------------------------------

				constexpr explicit file(const ca::span<const ca::byte> bytes) noexcept
				: _bytes{bytes} {
				}


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC The file in bytes; none when they do not hold an ELF64 little-endian
				   file whose tables fit. */
				[[nodiscard]] static auto from(ca::span<const ca::byte> bytes) noexcept -> ca::optional<self>;


				// -- accessors ----------------------------------------------

				/* @DOC All the bytes of the file. */
				[[nodiscard]] constexpr auto bytes(void) const noexcept -> ca::span<const ca::byte> {
					return _bytes;
				}

				[[nodiscard]] auto header(void) const noexcept -> const ca::elf::file_header& {
					return *reinterpret_cast<const ca::elf::file_header*>(_bytes.data());
				}

				/* @DOC The program headers, empty in a file that has none (an object file). */
				[[nodiscard]] auto segments(void) const noexcept -> ca::borrowed_slice<const ca::elf::program_header>;

				/* @DOC The section headers, empty in a file stripped of them. */
				[[nodiscard]] auto sections(void) const noexcept -> ca::borrowed_slice<const ca::elf::section_header>;


				// -- methods ------------------------------------------------

				/* @DOC The bytes of section in the file; empty for a no_bits section or one
				   past the end. */
				[[nodiscard]] auto contents(const ca::elf::section_header& section) const noexcept -> ca::span<const ca::byte>;

				/* @DOC The bytes of segment present in the file (its _file_size); empty past
				   the end. */
				[[nodiscard]] auto contents(const ca::elf::program_header& segment) const noexcept -> ca::span<const ca::byte>;

				/* @DOC The name of section, empty when the names table lacks it. */
				[[nodiscard]] auto name(const ca::elf::section_header& section) const noexcept -> ca::sv::str;

				/* @DOC The first section named name. */
				[[nodiscard]] auto find(ca::sv::str name) const noexcept -> ca::optional<const ca::elf::section_header&>;

				/* @DOC The section section's _link names (the string table of a symbol table). */
				[[nodiscard]] auto linked(const ca::elf::section_header& section) const noexcept -> ca::optional<const ca::elf::section_header&>;

				/* @DOC The NUL-terminated string at offset in string table strings, empty when
				   it does not end inside it. */
				[[nodiscard]] auto string(const ca::elf::section_header& strings, ca::u64 offset) const noexcept -> ca::sv::str;

				/* @DOC The entries of symbol table table (symbols or dynamic_symbols); empty
				   for another section, or one misaligned or past the end. */
				[[nodiscard]] auto symbols(const ca::elf::section_header& table) const noexcept -> ca::borrowed_slice<const ca::elf::symbol>;

		}; // class file

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_file_hpp
