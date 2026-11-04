#ifndef ca_system_trace_impl_debug_info_hpp
#define ca_system_trace_impl_debug_info_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/elf/symbol.hpp"
#include "3-system/trace/location.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- DEBUG INFO -----------------------------------------------------

		namespace impl {

			/* @DOC The sections of an ELF64 little-endian file symbolization reads, each a view
			   into the file, empty when absent or compressed: symbol tables and their strings,
			   and the DWARF 4 and 5 sections for units, lines, strings, addresses and ranges. */
			struct debug_image final {

					using self = ca::trace::impl::debug_image;

					ca::borrowed_slice<const ca::elf::symbol> _symtab;
					ca::span<const ca::byte> _symstr;
					ca::borrowed_slice<const ca::elf::symbol> _dynsym;
					ca::span<const ca::byte> _dynstr;
					ca::span<const ca::byte> _info;
					ca::span<const ca::byte> _abbrev;
					ca::span<const ca::byte> _line;
					ca::span<const ca::byte> _str;
					ca::span<const ca::byte> _line_str;
					ca::span<const ca::byte> _str_offsets;
					ca::span<const ca::byte> _addr;
					ca::span<const ca::byte> _rnglists;
					ca::span<const ca::byte> _ranges;

			}; // struct debug_image

			/* @DOC The sections of file, an ELF64 little-endian image; none when it is not one. */
			[[nodiscard]] auto read_image(ca::span<const ca::byte> file) noexcept -> ca::optional<ca::trace::impl::debug_image>;

			/* @DOC The functions address (a link-time address of image) is in, innermost first:
			   the inlined ones, then the function they were inlined into; how many it wrote into
			   out, 0 when image knows nothing of address. Names from DWARF, else from the symbol
			   tables; file and line from DWARF. */
			[[nodiscard]] auto locate(const ca::trace::impl::debug_image& image, ca::u64 address, ca::span<ca::trace::location> out) noexcept -> ca::usz;

			/* @DOC Writes a location's fields, for locate. */
			struct location_access final {

					using self = ca::trace::impl::location_access;

					static auto push_name(ca::trace::location& at, ca::sv::str name) noexcept -> void;
					static auto set_source(ca::trace::location& at, ca::sv::str root, ca::sv::str directory, ca::sv::str file, ca::u32 line) noexcept -> void;
					static auto set_offset(ca::trace::location& at, ca::usz offset) noexcept -> void;
					static auto set_inlined(ca::trace::location& at) noexcept -> void;

			}; // struct location_access

		} // namespace impl

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_impl_debug_info_hpp
