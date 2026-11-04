#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u16.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"
#include "3-system/elf/file.hpp"
#include "3-system/elf/section_header.hpp"
#include "3-system/elf/symbol.hpp"
#include "3-system/trace/impl/debug_info.hpp"


namespace {

	constexpr ca::u64 NONE = ~ca::u64{0U};
	constexpr ca::usz ABBREV_INDEX = 2048U;
	constexpr ca::usz CHAIN = 16U;
	constexpr ca::usz NAME_HOPS = 8U;

	constexpr ca::u64 TAG_CLASS = 0x02U;
	constexpr ca::u64 TAG_STRUCTURE = 0x13U;
	constexpr ca::u64 TAG_UNION = 0x17U;
	constexpr ca::u64 TAG_INLINED = 0x1DU;
	constexpr ca::u64 TAG_SUBPROGRAM = 0x2EU;
	constexpr ca::u64 TAG_NAMESPACE = 0x39U;

	constexpr ca::u64 AT_NAME = 0x03U;
	constexpr ca::u64 AT_STMT_LIST = 0x10U;
	constexpr ca::u64 AT_LOW_PC = 0x11U;
	constexpr ca::u64 AT_HIGH_PC = 0x12U;
	constexpr ca::u64 AT_COMP_DIR = 0x1BU;
	constexpr ca::u64 AT_ABSTRACT_ORIGIN = 0x31U;
	constexpr ca::u64 AT_SPECIFICATION = 0x47U;
	constexpr ca::u64 AT_RANGES = 0x55U;
	constexpr ca::u64 AT_CALL_FILE = 0x58U;
	constexpr ca::u64 AT_CALL_LINE = 0x59U;
	constexpr ca::u64 AT_STR_OFFSETS_BASE = 0x72U;
	constexpr ca::u64 AT_ADDR_BASE = 0x73U;
	constexpr ca::u64 AT_RNGLISTS_BASE = 0x74U;

	constexpr ca::u64 FORM_ADDR = 0x01U;
	constexpr ca::u64 FORM_BLOCK2 = 0x03U;
	constexpr ca::u64 FORM_BLOCK4 = 0x04U;
	constexpr ca::u64 FORM_DATA2 = 0x05U;
	constexpr ca::u64 FORM_DATA4 = 0x06U;
	constexpr ca::u64 FORM_DATA8 = 0x07U;
	constexpr ca::u64 FORM_STRING = 0x08U;
	constexpr ca::u64 FORM_BLOCK = 0x09U;
	constexpr ca::u64 FORM_BLOCK1 = 0x0AU;
	constexpr ca::u64 FORM_DATA1 = 0x0BU;
	constexpr ca::u64 FORM_FLAG = 0x0CU;
	constexpr ca::u64 FORM_SDATA = 0x0DU;
	constexpr ca::u64 FORM_STRP = 0x0EU;
	constexpr ca::u64 FORM_UDATA = 0x0FU;
	constexpr ca::u64 FORM_REF_ADDR = 0x10U;
	constexpr ca::u64 FORM_REF1 = 0x11U;
	constexpr ca::u64 FORM_REF2 = 0x12U;
	constexpr ca::u64 FORM_REF4 = 0x13U;
	constexpr ca::u64 FORM_REF8 = 0x14U;
	constexpr ca::u64 FORM_REF_UDATA = 0x15U;
	constexpr ca::u64 FORM_INDIRECT = 0x16U;
	constexpr ca::u64 FORM_SEC_OFFSET = 0x17U;
	constexpr ca::u64 FORM_EXPRLOC = 0x18U;
	constexpr ca::u64 FORM_FLAG_PRESENT = 0x19U;
	constexpr ca::u64 FORM_STRX = 0x1AU;
	constexpr ca::u64 FORM_ADDRX = 0x1BU;
	constexpr ca::u64 FORM_REF_SUP4 = 0x1CU;
	constexpr ca::u64 FORM_STRP_SUP = 0x1DU;
	constexpr ca::u64 FORM_DATA16 = 0x1EU;
	constexpr ca::u64 FORM_LINE_STRP = 0x1FU;
	constexpr ca::u64 FORM_REF_SIG8 = 0x20U;
	constexpr ca::u64 FORM_IMPLICIT_CONST = 0x21U;
	constexpr ca::u64 FORM_LOCLISTX = 0x22U;
	constexpr ca::u64 FORM_RNGLISTX = 0x23U;
	constexpr ca::u64 FORM_REF_SUP8 = 0x24U;
	constexpr ca::u64 FORM_STRX1 = 0x25U;
	constexpr ca::u64 FORM_STRX2 = 0x26U;
	constexpr ca::u64 FORM_STRX3 = 0x27U;
	constexpr ca::u64 FORM_STRX4 = 0x28U;
	constexpr ca::u64 FORM_ADDRX1 = 0x29U;
	constexpr ca::u64 FORM_ADDRX2 = 0x2AU;
	constexpr ca::u64 FORM_ADDRX3 = 0x2BU;
	constexpr ca::u64 FORM_ADDRX4 = 0x2CU;
	constexpr ca::u64 FORM_GNU_ADDR_INDEX = 0x1F01U;
	constexpr ca::u64 FORM_GNU_STR_INDEX = 0x1F02U;
	constexpr ca::u64 FORM_GNU_REF_ALT = 0x1F20U;
	constexpr ca::u64 FORM_GNU_STRP_ALT = 0x1F21U;

	constexpr ca::u64 LNCT_PATH = 0x1U;
	constexpr ca::u64 LNCT_DIRECTORY_INDEX = 0x2U;


	/* Reads a section: little-endian integers, LEB128, C strings. Reading past the end sets
	   _bad and yields 0, so malformed input ends a parse instead of faulting. */
	struct cursor final {

			using self = cursor;

			const ca::byte* _begin;
			const ca::byte* _at;
			const ca::byte* _end;
			bool _bad;

	}; // struct cursor

	auto cursor_at(const ca::span<const ca::byte> section, const ca::u64 offset) noexcept -> cursor {

		if (offset > section.size())
			return cursor{section.data(), section.data(), section.data(), true};
		return cursor{section.data(), section.data() + offset, section.data() + section.size(), false};
	}

	auto position(const cursor& c) noexcept -> ca::u64 {
		return static_cast<ca::u64>(c._at - c._begin);
	}

	auto at_end(const cursor& c) noexcept -> bool {
		return c._bad || c._at >= c._end;
	}

	auto skip(cursor& c, const ca::u64 count) noexcept -> void {

		if (c._bad || count > static_cast<ca::u64>(c._end - c._at)) {
			c._bad = true;
			return;
		}
		c._at += count;
	}

	auto read_fixed(cursor& c, const ca::usz size) noexcept -> ca::u64 {

		if (c._bad || size > 8U || size > static_cast<ca::usz>(c._end - c._at)) {
			c._bad = true;
			return 0U;
		}
		ca::u64 value = 0U;
		for (ca::usz i = 0U; i < size; ++i)
			value |= static_cast<ca::u64>(c._at[i]) << (8U * i);
		c._at += size;
		return value;
	}

	auto read_uleb(cursor& c) noexcept -> ca::u64 {

		ca::u64 value = 0U;
		ca::u32 shift = 0U;
		for (;;) {
			if (c._bad || c._at >= c._end) {
				c._bad = true;
				return 0U;
			}
			const ca::byte part = *c._at++;
			if (shift < 64U)
				value |= static_cast<ca::u64>(part & 0x7FU) << shift;
			shift += 7U;
			if ((part & 0x80U) == 0U)
				return value;
		}
	}

	auto read_sleb(cursor& c) noexcept -> ca::i64 {

		ca::u64 value = 0U;
		ca::u32 shift = 0U;
		for (;;) {
			if (c._bad || c._at >= c._end) {
				c._bad = true;
				return 0;
			}
			const ca::byte part = *c._at++;
			if (shift < 64U)
				value |= static_cast<ca::u64>(part & 0x7FU) << shift;
			shift += 7U;
			if ((part & 0x80U) == 0U) {
				if (shift < 64U && (part & 0x40U) != 0U)
					value |= ~ca::u64{0U} << shift;
				return static_cast<ca::i64>(value);
			}
		}
	}

	auto read_string(cursor& c) noexcept -> ca::sv::str {

		const ca::byte* const start = c._at;
		while (!c._bad && c._at < c._end && *c._at != 0U)
			++c._at;
		if (c._bad || c._at >= c._end) {
			c._bad = true;
			return ca::sv::str{};
		}
		const ca::sv::str text{reinterpret_cast<const char*>(start), static_cast<ca::usz>(c._at - start)};
		++c._at;
		return text;
	}

	auto string_in(const ca::span<const ca::byte> section, const ca::u64 offset) noexcept -> ca::sv::str {

		cursor c = cursor_at(section, offset);
		return read_string(c);
	}

	auto fixed_in(const ca::span<const ca::byte> section, const ca::u64 offset, const ca::usz size) noexcept -> ca::u64 {

		cursor c = cursor_at(section, offset);
		const ca::u64 value = read_fixed(c, size);
		return c._bad ? NONE : value;
	}


	/* A unit of .debug_info: where it is, how its values are encoded, and the bases its
	   compile-unit entry gives to indexed strings, addresses and range lists. */
	struct unit final {

			using self = unit;

			ca::u64 _offset;
			ca::u64 _end;
			ca::u64 _entries;
			ca::u64 _abbrev;
			ca::u64 _str_offsets_base;
			ca::u64 _addr_base;
			ca::u64 _rnglists_base;
			ca::u64 _base_address;
			ca::u16 _version;
			ca::u8 _address_size;
			ca::u8 _offset_size;
			bool _compile;

	}; // struct unit

	/* An attribute's value as encoded: its form, a number (constant, offset, index or address)
	   and, for strings and blocks, the bytes. */
	struct value final {

			using self = value;

			ca::u64 _form;
			ca::u64 _number;
			const ca::byte* _data;
			ca::usz _size;
			bool _present;

	}; // struct value

	/* The entries of .debug_info this reads, with the attributes symbolization needs. */
	struct entry final {

			using self = entry;

			ca::u64 _offset;
			ca::u64 _tag;
			bool _children;
			value _name;
			value _low;
			value _high;
			value _ranges;
			value _origin;
			value _specification;
			value _call_file;
			value _call_line;
			value _stmt_list;
			value _comp_dir;
			value _str_offsets_base;
			value _addr_base;
			value _rnglists_base;

	}; // struct entry

	/* The abbreviations of one unit, indexed by code (offset + 1 of the entry after its code;
	   0 for codes past the index, found by a scan). */
	struct abbreviations final {

			using self = abbreviations;

			ca::span<const ca::byte> _table;
			ca::u64 _unit;
			ca::u32 _index[ABBREV_INDEX];

	}; // struct abbreviations


	auto read_unit(const ca::trace::impl::debug_image& image, const ca::u64 offset, unit& out) noexcept -> bool {

		cursor c = cursor_at(image._info, offset);
		ca::u64 length = read_fixed(c, 4U);
		out._offset_size = 4U;
		if (length == 0xFFFFFFFFU) {
			          length = read_fixed(c, 8U);
			out._offset_size = 8U;
		}
		const ca::u64 start = position(c);
		 out._offset = offset;
		    out._end = start + length;
		out._version = static_cast<ca::u16>(read_fixed(c, 2U));
		out._compile = true;
		if (out._version >= 5U) {
			const ca::u64 type = read_fixed(c, 1U);
			out._address_size = static_cast<ca::u8>(read_fixed(c, 1U));
			      out._abbrev = read_fixed(c, out._offset_size);
			if (type == 2U || type == 6U)
				skip(c, 8U + out._offset_size);
			else if (type == 4U || type == 5U)
				skip(c, 8U);
			out._compile = type == 1U;
		}
		else {
			      out._abbrev = read_fixed(c, out._offset_size);
			out._address_size = static_cast<ca::u8>(read_fixed(c, 1U));
		}
		         out._entries = position(c);
		out._str_offsets_base = 0U;
		       out._addr_base = 0U;
		   out._rnglists_base = 0U;
		    out._base_address = 0U;
		return !c._bad && out._version >= 2U && out._version <= 5U && out._end <= image._info.size() && out._address_size <= 8U;
	}

	auto read_value(cursor& c, ca::u64 form, const ca::i64 implicit, const unit& u) noexcept -> value {

		value v{form, 0U, nullptr, 0U, true};
		if (form == FORM_INDIRECT) {
			   form = read_uleb(c);
			v._form = form;
		}
		switch (form) {
			case FORM_ADDR:
				v._number = read_fixed(c, u._address_size);
				break;
			case FORM_DATA1:
			case FORM_REF1:
			case FORM_FLAG:
			case FORM_STRX1:
			case FORM_ADDRX1:
				v._number = read_fixed(c, 1U);
				break;
			case FORM_DATA2:
			case FORM_REF2:
			case FORM_STRX2:
			case FORM_ADDRX2:
				v._number = read_fixed(c, 2U);
				break;
			case FORM_STRX3:
			case FORM_ADDRX3:
				v._number = read_fixed(c, 3U);
				break;
			case FORM_DATA4:
			case FORM_REF4:
			case FORM_REF_SUP4:
			case FORM_STRX4:
			case FORM_ADDRX4:
				v._number = read_fixed(c, 4U);
				break;
			case FORM_DATA8:
			case FORM_REF8:
			case FORM_REF_SIG8:
			case FORM_REF_SUP8:
				v._number = read_fixed(c, 8U);
				break;
			case FORM_DATA16:
				v._data = c._at;
				v._size = 16U;
				skip(c, 16U);
				break;
			case FORM_SDATA:
				v._number = static_cast<ca::u64>(read_sleb(c));
				break;
			case FORM_UDATA:
			case FORM_REF_UDATA:
			case FORM_STRX:
			case FORM_ADDRX:
			case FORM_LOCLISTX:
			case FORM_RNGLISTX:
			case FORM_GNU_ADDR_INDEX:
			case FORM_GNU_STR_INDEX:
				v._number = read_uleb(c);
				break;
			case FORM_STRP:
			case FORM_REF_ADDR:
			case FORM_SEC_OFFSET:
			case FORM_STRP_SUP:
			case FORM_LINE_STRP:
			case FORM_GNU_REF_ALT:
			case FORM_GNU_STRP_ALT:
				v._number = read_fixed(c, u._offset_size);
				break;
			case FORM_STRING: {
				const ca::sv::str text = read_string(c);
				v._data = reinterpret_cast<const ca::byte*>(text.data());
				v._size = text.size();
				break;
			}
			case FORM_BLOCK1:
			case FORM_BLOCK2:
			case FORM_BLOCK4:
			case FORM_BLOCK:
			case FORM_EXPRLOC: {
				const ca::u64 size = form == FORM_BLOCK1 ? read_fixed(c, 1U)
				                   : form == FORM_BLOCK2 ? read_fixed(c, 2U)
				                   : form == FORM_BLOCK4 ? read_fixed(c, 4U)
				                                         : read_uleb(c);
				v._data = c._at;
				v._size = static_cast<ca::usz>(size);
				skip(c, size);
				break;
			}
			case FORM_FLAG_PRESENT:
				v._number = 1U;
				break;
			case FORM_IMPLICIT_CONST:
				v._number = static_cast<ca::u64>(implicit);
				break;
			default:
				c._bad = true;
				break;
		}
		return v;
	}

	auto skip_specifications(cursor& c) noexcept -> void {
		for (;;) {
			const ca::u64 attribute = read_uleb(c);
			const ca::u64 form = read_uleb(c);
			if (form == FORM_IMPLICIT_CONST)
				static_cast<void>(read_sleb(c));
			if (c._bad || (attribute == 0U && form == 0U))
				return;
		}
	}

	auto load_abbreviations(const ca::trace::impl::debug_image& image, const unit& u, abbreviations& out) noexcept -> void {

		if (out._unit == u._offset)
			return;
		out._unit = u._offset;
		for (ca::usz i = 0U; i < ABBREV_INDEX; ++i)
			out._index[i] = 0U;
		out._table = u._abbrev <= image._abbrev.size() ? image._abbrev.without_prefix(static_cast<ca::usz>(u._abbrev)) : ca::span<const ca::byte>{};
		cursor c = cursor_at(out._table, 0U);
		for (;;) {
			const ca::u64 code = read_uleb(c);
			if (c._bad || code == 0U)
				return;
			if (code < ABBREV_INDEX)
				out._index[code] = static_cast<ca::u32>(position(c) + 1U);
			static_cast<void>(read_uleb(c));
			skip(c, 1U);
			skip_specifications(c);
		}
	}

	auto find_abbreviation(const abbreviations& table, const ca::u64 code) noexcept -> cursor {

		if (code < ABBREV_INDEX && table._index[code] != 0U)
			return cursor_at(table._table, table._index[code] - 1U);
		cursor c = cursor_at(table._table, 0U);
		for (;;) {
			const ca::u64 found = read_uleb(c);
			if (c._bad || found == 0U) {
				c._bad = true;
				return c;
			}
			if (found == code)
				return c;
			static_cast<void>(read_uleb(c));
			skip(c, 1U);
			skip_specifications(c);
		}
	}

	auto keep(entry& out, const ca::u64 attribute, const value& v) noexcept -> void {
		switch (attribute) {
			case AT_NAME:
				out._name = v;
				break;
			case AT_LOW_PC:
				out._low = v;
				break;
			case AT_HIGH_PC:
				out._high = v;
				break;
			case AT_RANGES:
				out._ranges = v;
				break;
			case AT_ABSTRACT_ORIGIN:
				out._origin = v;
				break;
			case AT_SPECIFICATION:
				out._specification = v;
				break;
			case AT_CALL_FILE:
				out._call_file = v;
				break;
			case AT_CALL_LINE:
				out._call_line = v;
				break;
			case AT_STMT_LIST:
				out._stmt_list = v;
				break;
			case AT_COMP_DIR:
				out._comp_dir = v;
				break;
			case AT_STR_OFFSETS_BASE:
				out._str_offsets_base = v;
				break;
			case AT_ADDR_BASE:
				out._addr_base = v;
				break;
			case AT_RNGLISTS_BASE:
				out._rnglists_base = v;
				break;
			default:
				break;
		}
	}

	/* Reads the entry at c: 1 for an entry, 0 for the null entry ending a list of children,
	   -1 when the input is malformed. */
	auto read_entry(cursor& c, const unit& u, const abbreviations& table, entry& out) noexcept -> int {

		        out = entry{};
		out._offset = position(c);
		const ca::u64 code = read_uleb(c);
		if (c._bad)
			return -1;
		if (code == 0U)
			return 0;
		cursor specification = find_abbreviation(table, code);
		     out._tag = read_uleb(specification);
		out._children = read_fixed(specification, 1U) != 0U;
		for (;;) {
			const ca::u64 attribute = read_uleb(specification);
			const ca::u64 form = read_uleb(specification);
			const ca::i64 implicit = form == FORM_IMPLICIT_CONST ? read_sleb(specification) : 0;
			if (specification._bad)
				return -1;
			if (attribute == 0U && form == 0U)
				break;
			const value v = read_value(c, form, implicit, u);
			if (c._bad)
				return -1;
			keep(out, attribute, v);
		}
		return 1;
	}

	auto is_constant(const value& v) noexcept -> bool {
		return v._form == FORM_DATA1 || v._form == FORM_DATA2 || v._form == FORM_DATA4 || v._form == FORM_DATA8
		    || v._form == FORM_SDATA || v._form == FORM_UDATA || v._form == FORM_IMPLICIT_CONST;
	}

	auto string_of(const ca::trace::impl::debug_image& image, const unit& u, const value& v) noexcept -> ca::sv::str {

		if (!v._present)
			return ca::sv::str{};
		switch (v._form) {
			case FORM_STRING:
				return ca::sv::str{reinterpret_cast<const char*>(v._data), v._size};
			case FORM_STRP:
				return string_in(image._str, v._number);
			case FORM_LINE_STRP:
				return string_in(image._line_str, v._number);
			case FORM_STRX:
			case FORM_STRX1:
			case FORM_STRX2:
			case FORM_STRX3:
			case FORM_STRX4:
			case FORM_GNU_STR_INDEX: {
				const ca::u64 offset = fixed_in(image._str_offsets, u._str_offsets_base + v._number * u._offset_size, u._offset_size);
				return offset == NONE ? ca::sv::str{} : string_in(image._str, offset);
			}
			default:
				return ca::sv::str{};
		}
	}

	auto address_at(const ca::trace::impl::debug_image& image, const unit& u, const ca::u64 index) noexcept -> ca::u64 {
		return fixed_in(image._addr, u._addr_base + index * u._address_size, u._address_size);
	}

	auto address_of(const ca::trace::impl::debug_image& image, const unit& u, const value& v) noexcept -> ca::u64 {

		if (!v._present)
			return NONE;
		switch (v._form) {
			case FORM_ADDR:
				return v._number;
			case FORM_ADDRX:
			case FORM_ADDRX1:
			case FORM_ADDRX2:
			case FORM_ADDRX3:
			case FORM_ADDRX4:
			case FORM_GNU_ADDR_INDEX:
				return address_at(image, u, v._number);
			default:
				return NONE;
		}
	}

	auto reference_of(const unit& u, const value& v) noexcept -> ca::u64 {

		if (!v._present)
			return NONE;
		switch (v._form) {
			case FORM_REF1:
			case FORM_REF2:
			case FORM_REF4:
			case FORM_REF8:
			case FORM_REF_UDATA:
				return u._offset + v._number;
			case FORM_REF_ADDR:
				return v._number;
			default:
				return NONE;
		}
	}

	auto in_range_list(const ca::trace::impl::debug_image& image, const unit& u, const value& ranges, const ca::u64 address) noexcept -> bool {

		ca::u64 base = u._base_address;
		if (u._version < 5U) {
			const ca::u64 all = u._address_size == 8U ? ~ca::u64{0U} : (ca::u64{1U} << (8U * u._address_size)) - 1U;
			cursor c = cursor_at(image._ranges, ranges._number);
			while (!c._bad) {
				const ca::u64 begin = read_fixed(c, u._address_size);
				const ca::u64 end = read_fixed(c, u._address_size);
				if (c._bad || (begin == 0U && end == 0U))
					return false;
				if (begin == all)
					base = end;
				else if (address >= base + begin && address < base + end)
					return true;
			}
			return false;
		}
		ca::u64 offset = ranges._number;
		if (ranges._form == FORM_RNGLISTX) {
			const ca::u64 relative = fixed_in(image._rnglists, u._rnglists_base + ranges._number * u._offset_size, u._offset_size);
			if (relative == NONE)
				return false;
			offset = u._rnglists_base + relative;
		}
		cursor c = cursor_at(image._rnglists, offset);
		while (!c._bad) {
			const ca::u64 kind = read_fixed(c, 1U);
			ca::u64 begin = 0U;
			ca::u64 end = 0U;
			switch (kind) {
				case 0U:
					return false;
				case 1U:
					base = address_at(image, u, read_uleb(c));
					continue;
				case 2U:
					begin = address_at(image, u, read_uleb(c));
					end = address_at(image, u, read_uleb(c));
					break;
				case 3U:
					begin = address_at(image, u, read_uleb(c));
					end = begin + read_uleb(c);
					break;
				case 4U:
					begin = base + read_uleb(c);
					end = base + read_uleb(c);
					break;
				case 5U:
					base = read_fixed(c, u._address_size);
					continue;
				case 6U:
					begin = read_fixed(c, u._address_size);
					end = read_fixed(c, u._address_size);
					break;
				case 7U:
					begin = read_fixed(c, u._address_size);
					end = begin + read_uleb(c);
					break;
				default:
					return false;
			}
			if (!c._bad && address >= begin && address < end)
				return true;
		}
		return false;
	}

	auto contains(const ca::trace::impl::debug_image& image, const unit& u, const entry& e, const ca::u64 address) noexcept -> bool {

		if (e._low._present && e._high._present) {
			const ca::u64 low = address_of(image, u, e._low);
			if (low == NONE)
				return false;
			const ca::u64 high = is_constant(e._high) ? low + e._high._number : address_of(image, u, e._high);
			return high != NONE && address >= low && address < high;
		}
		if (e._ranges._present)
			return in_range_list(image, u, e._ranges, address);
		return false;
	}

	/* Reads the unit at offset, its abbreviations and its root entry, which gives the unit its
	   bases. */
	auto open_unit(const ca::trace::impl::debug_image& image, const ca::u64 offset, unit& u, abbreviations& table, entry& root) noexcept -> bool {

		if (!read_unit(image, offset, u))
			return false;
		load_abbreviations(image, u, table);
		cursor c = cursor_at(image._info, u._entries);
		if (read_entry(c, u, table, root) != 1)
			return false;
		if (root._str_offsets_base._present)
			u._str_offsets_base = root._str_offsets_base._number;
		if (root._addr_base._present)
			u._addr_base = root._addr_base._number;
		if (root._rnglists_base._present)
			u._rnglists_base = root._rnglists_base._number;
		const ca::u64 base = address_of(image, u, root._low);
		u._base_address = base == NONE ? 0U : base;
		return true;
	}

	auto unit_holding(const ca::trace::impl::debug_image& image, const ca::u64 offset, unit& u, abbreviations& table, entry& root) noexcept -> bool {

		ca::u64 at = 0U;
		while (at < image._info.size()) {
			unit candidate{};
			if (!read_unit(image, at, candidate))
				return false;
			if (offset >= candidate._entries && offset < candidate._end)
				return open_unit(image, at, u, table, root);
			at = candidate._end;
		}
		return false;
	}

	auto entry_at(const ca::trace::impl::debug_image& image, const ca::u64 offset, unit& u, abbreviations& table, entry& out) noexcept -> bool {

		entry root{};
		if (!unit_holding(image, offset, u, table, root))
			return false;
		cursor c = cursor_at(image._info, offset);
		return read_entry(c, u, table, out) == 1;
	}

	auto is_scope(const ca::u64 tag) noexcept -> bool {
		return tag == TAG_NAMESPACE || tag == TAG_CLASS || tag == TAG_STRUCTURE || tag == TAG_UNION || tag == TAG_SUBPROGRAM;
	}

	auto unnamed(const ca::u64 tag) noexcept -> ca::sv::str {
		return tag == TAG_NAMESPACE ? ca::sv::str{"(anonymous namespace)"} : ca::sv::str{"(unnamed)"};
	}

	/* The name of e, following its specification or abstract origin while they stay in its
	   unit (whose abbreviations are loaded); unnamed(tag) when none. */
	auto scope_name(const ca::trace::impl::debug_image& image, const unit& u, const abbreviations& table, entry e) noexcept -> ca::sv::str {

		const ca::u64 tag = e._tag;
		for (ca::usz hop = 0U; hop < NAME_HOPS; ++hop) {
			if (e._name._present)
				return string_of(image, u, e._name);
			ca::u64 next = reference_of(u, e._specification);
			if (next == NONE)
				next = reference_of(u, e._origin);
			if (next == NONE || next < u._entries || next >= u._end)
				break;
			cursor c = cursor_at(image._info, next);
			if (read_entry(c, u, table, e) != 1)
				break;
		}
		return unnamed(tag);
	}

	/* Pushes the qualified name of the entry at target onto out: the scopes holding it, then
	   its own name. */
	auto qualify(const ca::trace::impl::debug_image& image, const ca::u64 target, abbreviations& table, ca::trace::location& out) noexcept -> bool {

		unit u{};
		entry root{};
		if (!unit_holding(image, target, u, table, root))
			return false;
		ca::sv::str names[ca::trace::location::SCOPES];
		ca::usz depths[ca::trace::location::SCOPES];
		ca::usz count = 0U;
		ca::usz depth = root._children ? 1U : 0U;
		cursor c = cursor_at(image._info, u._entries);
		entry e{};
		static_cast<void>(read_entry(c, u, table, e));
		while (depth > 0U && position(c) < u._end) {
			const int read = read_entry(c, u, table, e);
			if (read < 0)
				return false;
			if (read == 0) {
				--depth;
				continue;
			}
			while (count > 0U && depths[count - 1U] >= depth)
				--count;
			if (e._offset == target) {
				for (ca::usz i = 0U; i < count; ++i)
					ca::trace::impl::location_access::push_name(out, names[i]);
				const ca::sv::str name = string_of(image, u, e._name);
				ca::trace::impl::location_access::push_name(out, name.empty() ? unnamed(e._tag) : name);
				return true;
			}
			if (e._children && is_scope(e._tag) && count < ca::trace::location::SCOPES) {
				const ca::sv::str name = scope_name(image, u, table, e);
				 names[count] = name.empty() ? unnamed(e._tag) : name;
				depths[count] = depth;
				++count;
			}
			if (e._children)
				++depth;
		}
		return false;
	}

	/* Names the function of e: the entry holding its name, reached through the abstract
	   origin of an inlined or out-of-line instance and the declaration a definition
	   specifies. */
	auto name_function(const ca::trace::impl::debug_image& image, unit u, entry e, abbreviations& table, ca::trace::location& out) noexcept -> bool {

		for (ca::usz hop = 0U; hop < NAME_HOPS; ++hop) {
			if (e._name._present)
				return qualify(image, e._offset, table, out);
			ca::u64 next = reference_of(u, e._specification);
			if (next == NONE)
				next = reference_of(u, e._origin);
			if (next == NONE || !entry_at(image, next, u, table, e))
				return false;
		}
		return false;
	}


	/* A unit's line table: its header, where its directory and file entries start, and its
	   line program. */
	struct line_table final {

			using self = line_table;

			unit _unit;
			ca::u64 _version;
			ca::u64 _min_instruction;
			ca::i64 _line_base;
			ca::u64 _line_range;
			ca::u64 _opcode_base;
			const ca::byte* _standard_lengths;
			cursor _directory_format;
			ca::u64 _directory_format_count;
			cursor _directories;
			ca::u64 _directory_count;
			cursor _file_format;
			ca::u64 _file_format_count;
			cursor _files;
			ca::u64 _file_count;
			cursor _program;
			ca::sv::str _comp_dir;

	}; // struct line_table

	auto skip_formats(cursor& c, const ca::u64 count) noexcept -> void {
		for (ca::u64 i = 0U; i < count && !c._bad; ++i) {
			static_cast<void>(read_uleb(c));
			static_cast<void>(read_uleb(c));
		}
	}

	/* Reads one directory or file entry of a version 5 table at c: its path and directory
	   index. */
	auto read_path_entry(const ca::trace::impl::debug_image& image, const line_table& table, cursor& c, cursor format, const ca::u64 format_count, ca::sv::str& path, ca::u64& directory) noexcept -> void {
		for (ca::u64 i = 0U; i < format_count && !c._bad; ++i) {
			const ca::u64 content = read_uleb(format);
			const ca::u64 form = read_uleb(format);
			const value v = read_value(c, form, 0, table._unit);
			if (content == LNCT_PATH)
				path = string_of(image, table._unit, v);
			else if (content == LNCT_DIRECTORY_INDEX)
				directory = v._number;
		}
	}

	auto read_line_table(const ca::trace::impl::debug_image& image, const unit& u, const ca::u64 offset, const ca::sv::str comp_dir, line_table& out) noexcept -> bool {

		cursor c = cursor_at(image._line, offset);
		    out._unit = u;
		out._comp_dir = comp_dir;
		ca::u64 length = read_fixed(c, 4U);
		out._unit._offset_size = 4U;
		if (length == 0xFFFFFFFFU) {
			                length = read_fixed(c, 8U);
			out._unit._offset_size = 8U;
		}
		const ca::u64 end = position(c) + length;
		out._version = read_fixed(c, 2U);
		if (out._version >= 5U) {
			out._unit._address_size = static_cast<ca::u8>(read_fixed(c, 1U));
			skip(c, 1U);
		}
		const ca::u64 header_length = read_fixed(c, out._unit._offset_size);
		const ca::u64 program = position(c) + header_length;
		out._min_instruction = read_fixed(c, 1U);
		if (out._version >= 4U)
			skip(c, 1U);
		skip(c, 1U);
		       out._line_base = static_cast<ca::i64>(static_cast<signed char>(read_fixed(c, 1U)));
		      out._line_range = read_fixed(c, 1U);
		     out._opcode_base = read_fixed(c, 1U);
		out._standard_lengths = c._at;
		skip(c, out._opcode_base == 0U ? 0U : out._opcode_base - 1U);
		if (out._version >= 5U) {
			out._directory_format_count = read_fixed(c, 1U);
			      out._directory_format = c;
			skip_formats(c, out._directory_format_count);
			out._directory_count = read_uleb(c);
			    out._directories = c;
			for (ca::u64 i = 0U; i < out._directory_count && !c._bad; ++i) {
				ca::sv::str path{};
				ca::u64 directory = 0U;
				read_path_entry(image, out, c, out._directory_format, out._directory_format_count, path, directory);
			}
			out._file_format_count = read_fixed(c, 1U);
			      out._file_format = c;
			skip_formats(c, out._file_format_count);
			out._file_count = read_uleb(c);
			     out._files = c;
		}
		else {
			out._directories = c;
			while (!c._bad && !read_string(c).empty()) {
			}
			out._files = c;
		}
		     out._program = cursor_at(image._line, program);
		out._program._end = c._bad || end > image._line.size() ? out._program._at : image._line.data() + end;
		return !c._bad && !out._program._bad && out._line_range != 0U;
	}

	auto directory_name(const ca::trace::impl::debug_image& image, const line_table& table, const ca::u64 index) noexcept -> ca::sv::str {

		cursor c = table._directories;
		if (table._version >= 5U) {
			for (ca::u64 i = 0U; i < table._directory_count && !c._bad; ++i) {
				ca::sv::str path{};
				ca::u64 directory = 0U;
				read_path_entry(image, table, c, table._directory_format, table._directory_format_count, path, directory);
				if (i == index)
					return path;
			}
			return ca::sv::str{};
		}
		if (index == 0U)
			return table._comp_dir;
		for (ca::u64 i = 1U; !c._bad; ++i) {
			const ca::sv::str path = read_string(c);
			if (path.empty())
				return ca::sv::str{};
			if (i == index)
				return path;
		}
		return ca::sv::str{};
	}

	auto file_name(const ca::trace::impl::debug_image& image, const line_table& table, const ca::u64 index, ca::sv::str& directory, ca::sv::str& file) noexcept -> bool {

		cursor c = table._files;
		ca::u64 directory_index = 0U;
		if (table._version >= 5U) {
			for (ca::u64 i = 0U; i < table._file_count && !c._bad; ++i) {
				ca::sv::str path{};
				ca::u64 at = 0U;
				read_path_entry(image, table, c, table._file_format, table._file_format_count, path, at);
				if (i == index) {
					           file = path;
					directory_index = at;
					break;
				}
			}
		}
		else {
			for (ca::u64 i = 1U; !c._bad; ++i) {
				const ca::sv::str path = read_string(c);
				if (path.empty())
					return false;
				const ca::u64 at = read_uleb(c);
				static_cast<void>(read_uleb(c));
				static_cast<void>(read_uleb(c));
				if (i == index) {
					           file = path;
					directory_index = at;
					break;
				}
			}
		}
		if (c._bad || file.empty())
			return false;
		directory = file.data()[0] == '/' ? ca::sv::str{} : directory_name(image, table, directory_index);
		return true;
	}

	/* Runs the line program: the file and line of the row covering address. */
	auto find_line(const line_table& table, const ca::u64 address, ca::u64& file, ca::u32& line) noexcept -> bool {

		cursor c = table._program;
		ca::u64 at = 0U;
		ca::u64 row_file = 1U;
		ca::i64 row_line = 1;
		bool previous = false;
		ca::u64 previous_address = 0U;
		ca::u64 previous_file = 0U;
		ca::i64 previous_line = 0;
		const auto row = [&](void) noexcept -> bool {
			if (previous && previous_address <= address && address < at) {
				file = previous_file;
				line = static_cast<ca::u32>(previous_line);
				return true;
			}
			        previous = true;
			previous_address = at;
			   previous_file = row_file;
			   previous_line = row_line;
			return false;
		};
		while (!at_end(c)) {
			const ca::u64 opcode = read_fixed(c, 1U);
			if (opcode >= table._opcode_base) {
				const ca::u64 adjusted = opcode - table._opcode_base;
				at += table._min_instruction * (adjusted / table._line_range);
				row_line += table._line_base + static_cast<ca::i64>(adjusted % table._line_range);
				if (row())
					return true;
				continue;
			}
			switch (opcode) {
				case 0U: {
					const ca::u64 length = read_uleb(c);
					const ca::u64 start = position(c);
					const ca::u64 extended = read_fixed(c, 1U);
					if (extended == 1U) {
						if (previous && previous_address <= address && address < at) {
							file = previous_file;
							line = static_cast<ca::u32>(previous_line);
							return true;
						}
						previous = false;
						      at = 0U;
						row_file = 1U;
						row_line = 1;
					}
					else if (extended == 2U)
						at = read_fixed(c, static_cast<ca::usz>(length - 1U));
					c = cursor_at(ca::span<const ca::byte>{c._begin, static_cast<ca::usz>(c._end - c._begin)}, start + length);
					break;
				}
				case 1U:
					if (row())
						return true;
					break;
				case 2U:
					at += table._min_instruction * read_uleb(c);
					break;
				case 3U:
					row_line += read_sleb(c);
					break;
				case 4U:
					row_file = read_uleb(c);
					break;
				case 5U:
					static_cast<void>(read_uleb(c));
					break;
				case 8U:
					at += table._min_instruction * ((255U - table._opcode_base) / table._line_range);
					break;
				case 9U:
					at += read_fixed(c, 2U);
					break;
				case 6U:
				case 7U:
				case 10U:
				case 11U:
					break;
				default:
					for (ca::u64 i = 0U; i < table._standard_lengths[opcode - 1U]; ++i)
						static_cast<void>(read_uleb(c));
					break;
			}
		}
		return false;
	}

	auto set_source(const ca::trace::impl::debug_image& image, const line_table& table, const ca::u64 file, const ca::u32 line, ca::trace::location& out) noexcept -> void {

		ca::sv::str directory{};
		ca::sv::str name{};
		if (file_name(image, table, file, directory, name))
			ca::trace::impl::location_access::set_source(out, directory_name(image, table, 0U), directory, name, line);
	}

	auto find_symbol(const ca::borrowed_slice<const ca::elf::symbol> symbols, const ca::span<const ca::byte> strings, const ca::u64 address, ca::sv::str& name, ca::u64& start) noexcept -> bool {

		for (const ca::elf::symbol& symbol : symbols) {
			const ca::u8 type = symbol.type();
			if ((type == ca::elf::symbol_types::function || type == ca::elf::symbol_types::gnu_ifunc) && symbol.is_defined() && symbol._size != 0U
			 && address >= symbol._value && address - symbol._value < symbol._size) {
				 name = string_in(strings, symbol._name);
				start = symbol._value;
				return true;
			}
		}
		return false;
	}

	/* Locates address in the DWARF of image, writing the chain of functions holding it into
	   out, innermost first. */
	auto locate_in_units(const ca::trace::impl::debug_image& image, const ca::u64 address, ca::span<ca::trace::location> out, abbreviations& table) noexcept -> ca::usz {

		ca::u64 at = 0U;
		while (at < image._info.size()) {
			unit u{};
			entry root{};
			if (!open_unit(image, at, u, table, root))
				return 0U;
			at = u._end;
			if (!u._compile || !contains(image, u, root, address))
				continue;
			entry chain[CHAIN];
			ca::usz depths[CHAIN];
			ca::usz count = 0U;
			ca::usz depth = root._children ? 1U : 0U;
			cursor c = cursor_at(image._info, u._entries);
			entry e{};
			static_cast<void>(read_entry(c, u, table, e));
			while (depth > 0U && position(c) < u._end) {
				const int read = read_entry(c, u, table, e);
				if (read < 0)
					break;
				if (read == 0) {
					--depth;
					continue;
				}
				if (count > 0U && depth <= depths[0])
					break;
				if ((e._tag == TAG_SUBPROGRAM || e._tag == TAG_INLINED) && count < CHAIN && (count == 0U || depth > depths[count - 1U])
				 && contains(image, u, e, address)) {
					 chain[count] = e;
					depths[count] = depth;
					++count;
				}
				if (e._children)
					++depth;
			}
			line_table lines{};
			const bool has_lines = root._stmt_list._present && read_line_table(image, u, root._stmt_list._number, string_of(image, u, root._comp_dir), lines);
			ca::u64 file = 0U;
			ca::u32 line = 0U;
			const bool found_line = has_lines && find_line(lines, address, file, line);
			if (count == 0U) {
				if (found_line)
					set_source(image, lines, file, line, out[0]);
				return found_line ? 1U : 0U;
			}
			ca::usz written = 0U;
			for (; written < count && written < out.size(); ++written) {
				const ca::usz k = count - 1U - written;
				ca::trace::location& here = out[written];
				static_cast<void>(name_function(image, u, chain[k], table, here));
				if (written == 0U) {
					if (found_line)
						set_source(image, lines, file, line, here);
				}
				else if (has_lines && chain[k + 1U]._call_file._present)
					set_source(image, lines, chain[k + 1U]._call_file._number, static_cast<ca::u32>(chain[k + 1U]._call_line._number), here);
				if (chain[k]._tag == TAG_INLINED)
					ca::trace::impl::location_access::set_inlined(here);
			}
			return written;
		}
		return 0U;
	}

} // namespace


auto ca::trace::impl::location_access::push_name(ca::trace::location& at, const ca::sv::str name) noexcept -> void {

	if (at._count == ca::trace::location::SCOPES) {
		for (ca::usz i = 1U; i < ca::trace::location::SCOPES; ++i)
			at._names[i - 1U] = at._names[i];
		--at._count;
	}
	at._names[at._count++] = name;
}

auto ca::trace::impl::location_access::set_source(ca::trace::location& at, const ca::sv::str root, const ca::sv::str directory, const ca::sv::str file, const ca::u32 line) noexcept -> void {

	     at._root = root;
	at._directory = directory;
	     at._file = file;
	     at._line = line;
}

auto ca::trace::impl::location_access::set_offset(ca::trace::location& at, const ca::usz offset) noexcept -> void {

	    at._offset = offset;
	at._has_offset = true;
}

auto ca::trace::impl::location_access::set_inlined(ca::trace::location& at) noexcept -> void {
	at._inlined = true;
}

[[nodiscard]] auto ca::trace::impl::read_image(const ca::span<const ca::byte> file) noexcept -> ca::optional<ca::trace::impl::debug_image> {

	const auto elf = ca::elf::file::from(file);
	if (!elf)
		return ca::optional<ca::trace::impl::debug_image>{};
	ca::trace::impl::debug_image image{};
	for (const ca::elf::section_header& section : elf->sections()) {
		if (section._type == ca::elf::section_types::no_bits || (section._flags & ca::elf::section_flags::compressed) != 0U)
			continue;
		const ca::sv::str name = elf->name(section);
		const ca::span<const ca::byte> contents = elf->contents(section);
		if (section._type == ca::elf::section_types::symbols || section._type == ca::elf::section_types::dynamic_symbols) {
			const auto strings = elf->linked(section);
			if (!strings)
				continue;
			const bool full = section._type == ca::elf::section_types::symbols;
			(full ? image._symtab : image._dynsym) = elf->symbols(section);
			(full ? image._symstr : image._dynstr) = elf->contents(*strings);
		}
		else if (name == ca::sv::str{".debug_info"})
			image._info = contents;
		else if (name == ca::sv::str{".debug_abbrev"})
			image._abbrev = contents;
		else if (name == ca::sv::str{".debug_line"})
			image._line = contents;
		else if (name == ca::sv::str{".debug_str"})
			image._str = contents;
		else if (name == ca::sv::str{".debug_line_str"})
			image._line_str = contents;
		else if (name == ca::sv::str{".debug_str_offsets"})
			image._str_offsets = contents;
		else if (name == ca::sv::str{".debug_addr"})
			image._addr = contents;
		else if (name == ca::sv::str{".debug_rnglists"})
			image._rnglists = contents;
		else if (name == ca::sv::str{".debug_ranges"})
			image._ranges = contents;
	}
	return ca::optional<ca::trace::impl::debug_image>{image};
}

auto ca::trace::impl::locate(const ca::trace::impl::debug_image& image, const ca::u64 address, const ca::span<ca::trace::location> out) noexcept -> ca::usz {

	if (out.empty())
		return 0U;
	for (ca::trace::location& each : out)
		each = ca::trace::location{};
	ca::sv::str symbol{};
	ca::u64 start = 0U;
	const bool has_symbol = find_symbol(image._symtab, image._symstr, address, symbol, start)
	                     || find_symbol(image._dynsym, image._dynstr, address, symbol, start);
	abbreviations table{};
	table._unit = NONE;
	ca::usz count = image._info.empty() || image._abbrev.empty() ? 0U : locate_in_units(image, address, out, table);
	if (count == 0U && !has_symbol)
		return 0U;
	if (count == 0U)
		count = 1U;
	ca::trace::location& outermost = out[count - 1U];
	if (has_symbol) {
		if (outermost.function().empty())
			ca::trace::impl::location_access::push_name(outermost, symbol);
		ca::trace::impl::location_access::set_offset(outermost, static_cast<ca::usz>(address - start));
	}
	return count;
}
