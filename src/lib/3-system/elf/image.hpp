#ifndef ca_system_elf_image_hpp
#define ca_system_elf_image_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/elf/dynamic_entry.hpp"
#include "3-system/elf/file_header.hpp"
#include "3-system/elf/hashed_name.hpp"
#include "3-system/elf/program_header.hpp"
#include "3-system/elf/relocation.hpp"
#include "3-system/elf/symbol.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- IMAGE ----------------------------------------------------------

		/* @DOC An ELF64 object mapped for execution (the vDSO, the running program, a library a
		   loader mapped): its segments, its bias (where it sits minus where it was linked) and
		   what its dynamic section gives, read once. An address in the dynamic section is
		   taken as a link-time address when it lies in the range the load segments were
		   linked at, as already rebased when it lies where they are mapped (libc's loader
		   rewrites them so in the objects it loads), and as absent otherwise. Lookups go
		   through the GNU hash table when there is one, else the SysV one. Owns nothing; the
		   mapping must outlive it. */
		class image final {


			private:

				// -- private types ------------------------------------------

				using self = ca::elf::image;


				// -- private members ----------------------------------------

				ca::u64 _bias;
				ca::u64 _link_begin;
				ca::u64 _link_end;
				ca::borrowed_slice<const ca::elf::program_header> _segments;
				ca::borrowed_slice<const ca::elf::dynamic_entry> _dynamic;
				const char* _strings;
				ca::usz _strings_size;
				const ca::elf::symbol* _symbols;
				ca::usz _symbol_count;
				const ca::u32* _hash;
				const ca::u32* _gnu_hash;
				const ca::u16* _versions;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC No segments, nothing dynamic: every lookup finds nothing. */
				constexpr image(void) noexcept
				: _bias{0U}, _link_begin{0U}, _link_end{0U}, _segments{}, _dynamic{}, _strings{nullptr}, _strings_size{0U}, _symbols{nullptr},
				  _symbol_count{0U}, _hash{nullptr}, _gnu_hash{nullptr}, _versions{nullptr} {
				}

				/* @DOC The image whose file header is mapped at header, as the segment holding
				   file offset 0 maps it (the vDSO from auxiliary_types::vdso, a library a
				   loader mapped); none when it is not a valid ELF64 header or no segment maps
				   it. */
				[[nodiscard]] static auto at(const ca::elf::file_header& header) noexcept -> ca::optional<self>;

				/* @DOC The image of program headers segments, mapped with bias (an object whose
				   program headers and bias are known; the running program is program()). */
				[[nodiscard]] static auto of(ca::borrowed_slice<const ca::elf::program_header> segments, ca::u64 bias) noexcept -> self;

				/* @DOC The bias of the image whose program headers are mapped at segments,
				   from its program_headers segment; none without one (a static PIE, which
				   program() handles through its file header). */
				[[nodiscard]] static auto bias_of(ca::borrowed_slice<const ca::elf::program_header> segments) noexcept -> ca::optional<ca::u64>;

				/* @DOC The running program, from the program headers in the auxiliary vector
				   (auxiliary_types::program_headers, program_header_count). Its bias comes from
				   its program_headers segment, or, without one (a static PIE), from its ELF
				   header right before the program headers, read through the kernel and checked
				   (ELF64, program headers right after it) and the load segment at file offset
				   0. Its dynamic section is there only when the program has one: a static
				   non-PIE program has none (GNU ld makes none even with --export-dynamic),
				   and then dynamic() and symbols() are empty. None when the vector gives no
				   program headers or the bias cannot be found; Linux x86-64 only. Reads the
				   auxiliary vector at each call: keep the result, the program never moves. */
				[[nodiscard]] static auto program(void) noexcept -> ca::optional<self>;


				// -- accessors ----------------------------------------------

				/* @DOC Address in memory minus link-time address. */
				[[nodiscard]] constexpr auto bias(void) const noexcept -> ca::u64 {
					return _bias;
				}

				[[nodiscard]] constexpr auto segments(void) const noexcept -> ca::borrowed_slice<const ca::elf::program_header> {
					return _segments;
				}

				/* @DOC The dynamic section, up to its null entry excluded; empty in a static
				   non-PIE program. */
				[[nodiscard]] constexpr auto dynamic(void) const noexcept -> ca::borrowed_slice<const ca::elf::dynamic_entry> {
					return _dynamic;
				}

				/* @DOC The dynamic symbol table, as many entries as the hash tables index. */
				[[nodiscard]] constexpr auto symbols(void) const noexcept -> ca::borrowed_slice<const ca::elf::symbol> {
					return ca::borrowed_slice<const ca::elf::symbol>{_symbols, _symbol_count};
				}


				// -- methods ------------------------------------------------

				/* @DOC Where link-time address link_address is in memory. */
				[[nodiscard]] constexpr auto address(const ca::u64 link_address) const noexcept -> ca::u64 {
					return _bias + link_address;
				}

				/* @DOC The first segment of type (segment_types). */
				[[nodiscard]] auto segment(ca::u32 type) const noexcept -> ca::optional<const ca::elf::program_header&>;

				/* @DOC The value of the first dynamic entry tagged tag (dynamic_tags). */
				[[nodiscard]] auto value(ca::i64 tag) const noexcept -> ca::optional<ca::u64>;

				/* @DOC Where the address entry tagged tag points in memory (dynamic_tags::init,
				   fini, ...); none when absent or outside the image. */
				[[nodiscard]] auto address_of(ca::i64 tag) const noexcept -> ca::optional<ca::u64>;

				/* @DOC The words of the table the dynamic section gives by address_tag and
				   size_tag (init_array and init_array_size, fini_array, preinit_array). */
				[[nodiscard]] auto words(ca::i64 address_tag, ca::i64 size_tag) const noexcept -> ca::borrowed_slice<const ca::u64>;

				/* @DOC The string at offset in the dynamic string table, empty past it. */
				[[nodiscard]] auto string(ca::u64 offset) const noexcept -> ca::sv::str;

				/* @DOC The symbol named name the image defines, a hidden version (one only a
				   versioned reference may bind to) excluded. */
				[[nodiscard]] auto find(const ca::elf::hashed_name& name) const noexcept -> ca::optional<const ca::elf::symbol&>;

				/* @DOC The relocations of dynamic_tags::rela. */
				[[nodiscard]] auto relocations(void) const noexcept -> ca::borrowed_slice<const ca::elf::relocation>;

				/* @DOC The relocations of the PLT (dynamic_tags::plt_relocations), when they
				   carry addends. */
				[[nodiscard]] auto plt_relocations(void) const noexcept -> ca::borrowed_slice<const ca::elf::relocation>;

				/* @DOC The packed relative relocations (dynamic_tags::relr): an even word is
				   the link-time address of a word to add the bias to, an odd one a bitmap of
				   the 63 words after the last address. */
				[[nodiscard]] auto relative_relocations(void) const noexcept -> ca::borrowed_slice<const ca::u64>;


			private:

				// -- private methods ----------------------------------------

				/* Where the dynamic entry value of an address points: bias + value for a
				   link-time address, value itself for one a loader already rewrote, 0 for one
				   inside neither range. */
				[[nodiscard]] auto _pointer(ca::u64 value) const noexcept -> ca::u64;

				[[nodiscard]] auto _table(ca::i64 address_tag, ca::i64 size_tag, ca::usz entry_size) const noexcept -> ca::borrowed_slice<const ca::byte>;

		}; // class image

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_image_hpp
