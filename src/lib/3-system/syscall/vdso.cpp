#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/syscall/auxiliary.hpp"
#include "3-system/syscall/vdso.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)
#	include <elf.h>
#endif


#if ca_platform(linux) && ca_architecture(x86_64)

static auto same_name(const char* a, const char* b) noexcept -> bool {

	for (; *a != '\0' && *a == *b; ++a, ++b) {
	}
	return *a == *b;
}

/* Symbols in the table: nchain of DT_HASH, else one past the last bucket chain of DT_GNU_HASH. */
static auto symbol_count(const ca::u32* const hash, const ca::u32* const gnu_hash) noexcept -> ca::usz {

	if (hash != nullptr)
		return hash[1];
	if (gnu_hash == nullptr)
		return 0U;
	const ca::u32 buckets = gnu_hash[0];
	const ca::u32 offset = gnu_hash[1];
	const ca::u32 words = gnu_hash[2];
	const ca::u32* const bucket = gnu_hash + 4U + 2U * words;
	const ca::u32* const chain = bucket + buckets;
	ca::u32 last = 0U;
	for (ca::u32 i = 0U; i < buckets; ++i) {
		if (bucket[i] > last)
			last = bucket[i];
	}
	if (last < offset)
		return offset;
	while ((chain[last - offset] & 1U) == 0U)
		++last;
	return last + 1U;
}

[[nodiscard]] auto ca::syscall::vdso_symbol(const char* const name) noexcept -> void* {

	const ca::u64 base = ca::syscall::auxiliary(AT_SYSINFO_EHDR);
	if (base == 0U) [[unlikely]]
		return nullptr;
	const Elf64_Ehdr* const header = reinterpret_cast<const Elf64_Ehdr*>(base);
	const Elf64_Phdr* const segments = reinterpret_cast<const Elf64_Phdr*>(base + header->e_phoff);
	ca::u64 bias = base;
	for (ca::usz i = 0U; i < header->e_phnum; ++i) {
		if (segments[i].p_type == PT_LOAD && segments[i].p_offset == 0U)
			bias = base - segments[i].p_vaddr;
	}
	const Elf64_Dyn* dynamic = nullptr;
	for (ca::usz i = 0U; i < header->e_phnum; ++i) {
		if (segments[i].p_type == PT_DYNAMIC)
			dynamic = reinterpret_cast<const Elf64_Dyn*>(bias + segments[i].p_vaddr);
	}
	if (dynamic == nullptr) [[unlikely]]
		return nullptr;
	const char* strings = nullptr;
	const Elf64_Sym* symbols = nullptr;
	const ca::u32* hash = nullptr;
	const ca::u32* gnu_hash = nullptr;
	for (; dynamic->d_tag != DT_NULL; ++dynamic) {
		const ca::u64 at = bias + dynamic->d_un.d_ptr;
		if (dynamic->d_tag == DT_STRTAB)
			strings = reinterpret_cast<const char*>(at);
		else if (dynamic->d_tag == DT_SYMTAB)
			symbols = reinterpret_cast<const Elf64_Sym*>(at);
		else if (dynamic->d_tag == DT_HASH)
			hash = reinterpret_cast<const ca::u32*>(at);
		else if (dynamic->d_tag == DT_GNU_HASH)
			gnu_hash = reinterpret_cast<const ca::u32*>(at);
	}
	if (strings == nullptr || symbols == nullptr) [[unlikely]]
		return nullptr;
	const ca::usz count = symbol_count(hash, gnu_hash);
	for (ca::usz i = 0U; i < count; ++i) {
		const Elf64_Sym& symbol = symbols[i];
		if (ELF64_ST_TYPE(symbol.st_info) != STT_FUNC || symbol.st_shndx == SHN_UNDEF)
			continue;
		if (same_name(strings + symbol.st_name, name))
			return reinterpret_cast<void*>(bias + symbol.st_value);
	}
	return nullptr;
}

#else

[[nodiscard]] auto ca::syscall::vdso_symbol(const char* const name) noexcept -> void* {

	static_cast<void>(name);
	return nullptr;
}

#endif
