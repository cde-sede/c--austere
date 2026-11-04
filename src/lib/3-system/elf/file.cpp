#include "1-primitives/integers/usz.hpp"
#include "3-system/elf/file.hpp"


namespace {

	auto fits(const ca::span<const ca::byte> bytes, const ca::u64 offset, const ca::u64 size) noexcept -> bool {
		return offset <= bytes.size() && size <= bytes.size() - offset;
	}

	template <typename T>
	auto table(const ca::span<const ca::byte> bytes, const ca::u64 offset, const ca::u64 count) noexcept -> ca::borrowed_slice<const T> {

		if (count == 0U || offset % alignof(T) != 0U || offset > bytes.size() || count > (bytes.size() - offset) / sizeof(T))
			return ca::borrowed_slice<const T>{};
		return ca::borrowed_slice<const T>{reinterpret_cast<const T*>(bytes.data() + offset), static_cast<ca::usz>(count)};
	}

	auto string_at(const ca::span<const ca::byte> bytes, const ca::u64 offset) noexcept -> ca::sv::str {

		if (offset >= bytes.size())
			return ca::sv::str{};
		const char* const begin = reinterpret_cast<const char*>(bytes.data() + offset);
		const ca::usz room = bytes.size() - static_cast<ca::usz>(offset);
		for (ca::usz length = 0U; length < room; ++length) {
			if (begin[length] == '\0')
				return ca::sv::str{begin, length};
		}
		return ca::sv::str{};
	}

} // namespace


[[nodiscard]] auto ca::elf::file::from(const ca::span<const ca::byte> bytes) noexcept -> ca::optional<ca::elf::file> {

	if (bytes.size() < sizeof(ca::elf::file_header) || reinterpret_cast<ca::usz>(bytes.data()) % alignof(ca::elf::file_header) != 0U)
		return ca::optional<self>{};
	const auto& header = *reinterpret_cast<const ca::elf::file_header*>(bytes.data());
	if (!header.is_elf64())
		return ca::optional<self>{};
	if (header._segment_count != 0U && table<ca::elf::program_header>(bytes, header._segments_offset, header._segment_count).empty())
		return ca::optional<self>{};
	if (header._section_count != 0U && table<ca::elf::section_header>(bytes, header._sections_offset, header._section_count).empty())
		return ca::optional<self>{};
	return ca::optional<self>{self{bytes}};
}

[[nodiscard]] auto ca::elf::file::segments(void) const noexcept -> ca::borrowed_slice<const ca::elf::program_header> {
	return table<ca::elf::program_header>(_bytes, this->header()._segments_offset, this->header()._segment_count);
}

[[nodiscard]] auto ca::elf::file::sections(void) const noexcept -> ca::borrowed_slice<const ca::elf::section_header> {
	return table<ca::elf::section_header>(_bytes, this->header()._sections_offset, this->header()._section_count);
}

[[nodiscard]] auto ca::elf::file::contents(const ca::elf::section_header& section) const noexcept -> ca::span<const ca::byte> {

	if (section._type == ca::elf::section_types::no_bits || !fits(_bytes, section._offset, section._size))
		return ca::span<const ca::byte>{};
	return ca::span<const ca::byte>{_bytes.data() + section._offset, static_cast<ca::usz>(section._size)};
}

[[nodiscard]] auto ca::elf::file::contents(const ca::elf::program_header& segment) const noexcept -> ca::span<const ca::byte> {

	if (!fits(_bytes, segment._offset, segment._file_size))
		return ca::span<const ca::byte>{};
	return ca::span<const ca::byte>{_bytes.data() + segment._offset, static_cast<ca::usz>(segment._file_size)};
}

[[nodiscard]] auto ca::elf::file::name(const ca::elf::section_header& section) const noexcept -> ca::sv::str {

	const auto all = this->sections();
	const ca::usz names = this->header()._section_names;
	if (names >= all.size())
		return ca::sv::str{};
	return this->string(all[names], section._name);
}

[[nodiscard]] auto ca::elf::file::find(const ca::sv::str name) const noexcept -> ca::optional<const ca::elf::section_header&> {

	for (const ca::elf::section_header& section : this->sections()) {
		if (this->name(section) == name)
			return ca::optional<const ca::elf::section_header&>{section};
	}
	return ca::optional<const ca::elf::section_header&>{};
}

[[nodiscard]] auto ca::elf::file::linked(const ca::elf::section_header& section) const noexcept -> ca::optional<const ca::elf::section_header&> {

	const auto all = this->sections();
	if (section._link == 0U || section._link >= all.size())
		return ca::optional<const ca::elf::section_header&>{};
	return ca::optional<const ca::elf::section_header&>{all[section._link]};
}

[[nodiscard]] auto ca::elf::file::string(const ca::elf::section_header& strings, const ca::u64 offset) const noexcept -> ca::sv::str {
	return string_at(this->contents(strings), offset);
}

[[nodiscard]] auto ca::elf::file::symbols(const ca::elf::section_header& table) const noexcept -> ca::borrowed_slice<const ca::elf::symbol> {

	if (table._type != ca::elf::section_types::symbols && table._type != ca::elf::section_types::dynamic_symbols)
		return ca::borrowed_slice<const ca::elf::symbol>{};
	return ::table<ca::elf::symbol>(_bytes, table._offset, table._size / sizeof(ca::elf::symbol));
}
