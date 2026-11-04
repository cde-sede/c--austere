// expect: exit 0
#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/elf/file.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/memory/mmap_file.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/memory/sharing.hpp"
#include "6-resources/fs/descriptor.hpp"

[[gnu::noinline]] auto ca_test_marker(void) noexcept -> int {
	return 42;
}

static auto check(const ca::span<const ca::byte> bytes) noexcept -> int {
	const auto elf = ca::elf::file::from(bytes);
	if (!elf)
		return 10;
	const ca::elf::file_header& header = elf->header();
	if (header._machine != ca::elf::machines::x86_64)
		return 11;
	if (header._type != ca::elf::file_types::executable && header._type != ca::elf::file_types::shared)
		return 12;
	if (elf->segments().empty() || elf->sections().empty())
		return 13;
	bool loads = false;
	for (const ca::elf::program_header& segment : elf->segments())
		loads = loads || segment._type == ca::elf::segment_types::load;
	if (!loads)
		return 14;
	const auto symtab = elf->find(ca::sv::str{".symtab"});
	if (!symtab || symtab->_type != ca::elf::section_types::symbols)
		return 15;
	const auto strings = elf->linked(*symtab);
	if (!strings || elf->name(*strings) != ca::sv::str{".strtab"})
		return 16;
	bool found = false;
	for (const ca::elf::symbol& symbol : elf->symbols(*symtab)) {
		if (elf->string(*strings, symbol._name) == ca::sv::str{"_Z14ca_test_markerv"})
			found = symbol.type() == ca::elf::symbol_types::function && symbol.is_defined() && symbol._size != 0U;
	}
	if (!found)
		return 17;
	if (elf->find(ca::sv::str{".no_such_section"}) || !elf->symbols(*strings).empty())
		return 18;
	if (ca::elf::file::from(bytes.prefix(sizeof(ca::elf::file_header) - 1U)) || ca::elf::file::from(bytes.prefix(sizeof(ca::elf::file_header))))
		return 19;
	if (ca::elf::file::from(ca::span<const ca::byte>{bytes.data() + 1, bytes.size() - 1U}))
		return 20;
	return ca_test_marker() == 42 ? 0 : 21;
}

auto main(void) -> int {
	auto file = ca::fs::descriptor::open("/proc/self/exe");
	if (!file)
		return 1;
	auto status = ca::sys::fstat(file->native());
	if (!status)
		return 2;
	auto mapping = ca::sys::mmap_file(file->native(), 0U, static_cast<ca::usz>(status->size()), ca::sys::protection::read, ca::sys::sharing::copy_on_write);
	if (!file->cleanup() || !mapping)
		return 3;
	const int result = check(ca::span<const ca::byte>{mapping->data(), mapping->size()});
	return ca::sys::munmap(*mapping) ? result : 4;
}
