#include "3-system/elf/auxiliary_entry.hpp"
#include "3-system/elf/dynamic_entry.hpp"
#include "3-system/elf/file_header.hpp"
#include "3-system/elf/program_header.hpp"
#include "3-system/elf/relocation.hpp"
#include "3-system/elf/section_header.hpp"
#include "3-system/elf/symbol.hpp"

#include <elf.h>


static_assert(sizeof(ca::elf::file_header) == sizeof(Elf64_Ehdr) && alignof(ca::elf::file_header) == alignof(Elf64_Ehdr));
static_assert(__builtin_offsetof(ca::elf::file_header, _identity) == __builtin_offsetof(Elf64_Ehdr, e_ident));
static_assert(__builtin_offsetof(ca::elf::file_header, _type) == __builtin_offsetof(Elf64_Ehdr, e_type));
static_assert(__builtin_offsetof(ca::elf::file_header, _machine) == __builtin_offsetof(Elf64_Ehdr, e_machine));
static_assert(__builtin_offsetof(ca::elf::file_header, _version) == __builtin_offsetof(Elf64_Ehdr, e_version));
static_assert(__builtin_offsetof(ca::elf::file_header, _entry) == __builtin_offsetof(Elf64_Ehdr, e_entry));
static_assert(__builtin_offsetof(ca::elf::file_header, _segments_offset) == __builtin_offsetof(Elf64_Ehdr, e_phoff));
static_assert(__builtin_offsetof(ca::elf::file_header, _sections_offset) == __builtin_offsetof(Elf64_Ehdr, e_shoff));
static_assert(__builtin_offsetof(ca::elf::file_header, _flags) == __builtin_offsetof(Elf64_Ehdr, e_flags));
static_assert(__builtin_offsetof(ca::elf::file_header, _header_size) == __builtin_offsetof(Elf64_Ehdr, e_ehsize));
static_assert(__builtin_offsetof(ca::elf::file_header, _segment_size) == __builtin_offsetof(Elf64_Ehdr, e_phentsize));
static_assert(__builtin_offsetof(ca::elf::file_header, _segment_count) == __builtin_offsetof(Elf64_Ehdr, e_phnum));
static_assert(__builtin_offsetof(ca::elf::file_header, _section_size) == __builtin_offsetof(Elf64_Ehdr, e_shentsize));
static_assert(__builtin_offsetof(ca::elf::file_header, _section_count) == __builtin_offsetof(Elf64_Ehdr, e_shnum));
static_assert(__builtin_offsetof(ca::elf::file_header, _section_names) == __builtin_offsetof(Elf64_Ehdr, e_shstrndx));

static_assert(sizeof(ca::elf::program_header) == sizeof(Elf64_Phdr) && alignof(ca::elf::program_header) == alignof(Elf64_Phdr));
static_assert(__builtin_offsetof(ca::elf::program_header, _type) == __builtin_offsetof(Elf64_Phdr, p_type));
static_assert(__builtin_offsetof(ca::elf::program_header, _flags) == __builtin_offsetof(Elf64_Phdr, p_flags));
static_assert(__builtin_offsetof(ca::elf::program_header, _offset) == __builtin_offsetof(Elf64_Phdr, p_offset));
static_assert(__builtin_offsetof(ca::elf::program_header, _address) == __builtin_offsetof(Elf64_Phdr, p_vaddr));
static_assert(__builtin_offsetof(ca::elf::program_header, _physical) == __builtin_offsetof(Elf64_Phdr, p_paddr));
static_assert(__builtin_offsetof(ca::elf::program_header, _file_size) == __builtin_offsetof(Elf64_Phdr, p_filesz));
static_assert(__builtin_offsetof(ca::elf::program_header, _memory_size) == __builtin_offsetof(Elf64_Phdr, p_memsz));
static_assert(__builtin_offsetof(ca::elf::program_header, _align) == __builtin_offsetof(Elf64_Phdr, p_align));

static_assert(sizeof(ca::elf::section_header) == sizeof(Elf64_Shdr) && alignof(ca::elf::section_header) == alignof(Elf64_Shdr));
static_assert(__builtin_offsetof(ca::elf::section_header, _name) == __builtin_offsetof(Elf64_Shdr, sh_name));
static_assert(__builtin_offsetof(ca::elf::section_header, _type) == __builtin_offsetof(Elf64_Shdr, sh_type));
static_assert(__builtin_offsetof(ca::elf::section_header, _flags) == __builtin_offsetof(Elf64_Shdr, sh_flags));
static_assert(__builtin_offsetof(ca::elf::section_header, _address) == __builtin_offsetof(Elf64_Shdr, sh_addr));
static_assert(__builtin_offsetof(ca::elf::section_header, _offset) == __builtin_offsetof(Elf64_Shdr, sh_offset));
static_assert(__builtin_offsetof(ca::elf::section_header, _size) == __builtin_offsetof(Elf64_Shdr, sh_size));
static_assert(__builtin_offsetof(ca::elf::section_header, _link) == __builtin_offsetof(Elf64_Shdr, sh_link));
static_assert(__builtin_offsetof(ca::elf::section_header, _info) == __builtin_offsetof(Elf64_Shdr, sh_info));
static_assert(__builtin_offsetof(ca::elf::section_header, _align) == __builtin_offsetof(Elf64_Shdr, sh_addralign));
static_assert(__builtin_offsetof(ca::elf::section_header, _entry_size) == __builtin_offsetof(Elf64_Shdr, sh_entsize));

static_assert(sizeof(ca::elf::symbol) == sizeof(Elf64_Sym) && alignof(ca::elf::symbol) == alignof(Elf64_Sym));
static_assert(__builtin_offsetof(ca::elf::symbol, _name) == __builtin_offsetof(Elf64_Sym, st_name));
static_assert(__builtin_offsetof(ca::elf::symbol, _info) == __builtin_offsetof(Elf64_Sym, st_info));
static_assert(__builtin_offsetof(ca::elf::symbol, _other) == __builtin_offsetof(Elf64_Sym, st_other));
static_assert(__builtin_offsetof(ca::elf::symbol, _section) == __builtin_offsetof(Elf64_Sym, st_shndx));
static_assert(__builtin_offsetof(ca::elf::symbol, _value) == __builtin_offsetof(Elf64_Sym, st_value));
static_assert(__builtin_offsetof(ca::elf::symbol, _size) == __builtin_offsetof(Elf64_Sym, st_size));

static_assert(sizeof(ca::elf::dynamic_entry) == sizeof(Elf64_Dyn) && alignof(ca::elf::dynamic_entry) == alignof(Elf64_Dyn));
static_assert(__builtin_offsetof(ca::elf::dynamic_entry, _tag) == __builtin_offsetof(Elf64_Dyn, d_tag));
static_assert(__builtin_offsetof(ca::elf::dynamic_entry, _value) == __builtin_offsetof(Elf64_Dyn, d_un));

static_assert(sizeof(ca::elf::relocation) == sizeof(Elf64_Rela) && alignof(ca::elf::relocation) == alignof(Elf64_Rela));
static_assert(__builtin_offsetof(ca::elf::relocation, _offset) == __builtin_offsetof(Elf64_Rela, r_offset));
static_assert(__builtin_offsetof(ca::elf::relocation, _info) == __builtin_offsetof(Elf64_Rela, r_info));
static_assert(__builtin_offsetof(ca::elf::relocation, _addend) == __builtin_offsetof(Elf64_Rela, r_addend));

static_assert(sizeof(ca::elf::auxiliary_entry) == sizeof(Elf64_auxv_t) && alignof(ca::elf::auxiliary_entry) == alignof(Elf64_auxv_t));
static_assert(__builtin_offsetof(ca::elf::auxiliary_entry, _type) == __builtin_offsetof(Elf64_auxv_t, a_type));
static_assert(__builtin_offsetof(ca::elf::auxiliary_entry, _value) == __builtin_offsetof(Elf64_auxv_t, a_un));

static_assert(ca::elf::identity::size == EI_NIDENT);
static_assert(ca::elf::identity::class_index == EI_CLASS);
static_assert(ca::elf::identity::data_index == EI_DATA);
static_assert(ca::elf::identity::version_index == EI_VERSION);
static_assert(ca::elf::identity::abi_index == EI_OSABI);
static_assert(ca::elf::identity::class_64 == ELFCLASS64);
static_assert(ca::elf::identity::little_endian == ELFDATA2LSB);
static_assert(ca::elf::identity::current == EV_CURRENT);

static_assert(ca::elf::file_types::none == ET_NONE);
static_assert(ca::elf::file_types::relocatable == ET_REL);
static_assert(ca::elf::file_types::executable == ET_EXEC);
static_assert(ca::elf::file_types::shared == ET_DYN);
static_assert(ca::elf::file_types::core == ET_CORE);

static_assert(ca::elf::machines::x86_64 == EM_X86_64);
static_assert(ca::elf::machines::aarch64 == EM_AARCH64);
static_assert(ca::elf::machines::riscv == EM_RISCV);

static_assert(ca::elf::segment_types::null == PT_NULL);
static_assert(ca::elf::segment_types::load == PT_LOAD);
static_assert(ca::elf::segment_types::dynamic == PT_DYNAMIC);
static_assert(ca::elf::segment_types::interpreter == PT_INTERP);
static_assert(ca::elf::segment_types::note == PT_NOTE);
static_assert(ca::elf::segment_types::program_headers == PT_PHDR);
static_assert(ca::elf::segment_types::tls == PT_TLS);
static_assert(ca::elf::segment_types::gnu_eh_frame == PT_GNU_EH_FRAME);
static_assert(ca::elf::segment_types::gnu_stack == PT_GNU_STACK);
static_assert(ca::elf::segment_types::gnu_relro == PT_GNU_RELRO);
static_assert(ca::elf::segment_types::gnu_property == PT_GNU_PROPERTY);
static_assert(ca::elf::segment_flags::execute == PF_X);
static_assert(ca::elf::segment_flags::write == PF_W);
static_assert(ca::elf::segment_flags::read == PF_R);

static_assert(ca::elf::section_types::null == SHT_NULL);
static_assert(ca::elf::section_types::program_bits == SHT_PROGBITS);
static_assert(ca::elf::section_types::symbols == SHT_SYMTAB);
static_assert(ca::elf::section_types::strings == SHT_STRTAB);
static_assert(ca::elf::section_types::rela == SHT_RELA);
static_assert(ca::elf::section_types::hash == SHT_HASH);
static_assert(ca::elf::section_types::dynamic == SHT_DYNAMIC);
static_assert(ca::elf::section_types::note == SHT_NOTE);
static_assert(ca::elf::section_types::no_bits == SHT_NOBITS);
static_assert(ca::elf::section_types::rel == SHT_REL);
static_assert(ca::elf::section_types::dynamic_symbols == SHT_DYNSYM);
static_assert(ca::elf::section_types::init_array == SHT_INIT_ARRAY);
static_assert(ca::elf::section_types::fini_array == SHT_FINI_ARRAY);
static_assert(ca::elf::section_types::gnu_hash == SHT_GNU_HASH);
static_assert(ca::elf::section_flags::write == SHF_WRITE);
static_assert(ca::elf::section_flags::alloc == SHF_ALLOC);
static_assert(ca::elf::section_flags::execute == SHF_EXECINSTR);
static_assert(ca::elf::section_flags::tls == SHF_TLS);
static_assert(ca::elf::section_flags::compressed == SHF_COMPRESSED);
static_assert(ca::elf::section_indices::undefined == SHN_UNDEF);
static_assert(ca::elf::section_indices::absolute == SHN_ABS);
static_assert(ca::elf::section_indices::common == SHN_COMMON);

static_assert(ca::elf::symbol_types::none == STT_NOTYPE);
static_assert(ca::elf::symbol_types::object == STT_OBJECT);
static_assert(ca::elf::symbol_types::function == STT_FUNC);
static_assert(ca::elf::symbol_types::section == STT_SECTION);
static_assert(ca::elf::symbol_types::file == STT_FILE);
static_assert(ca::elf::symbol_types::common == STT_COMMON);
static_assert(ca::elf::symbol_types::tls == STT_TLS);
static_assert(ca::elf::symbol_types::gnu_ifunc == STT_GNU_IFUNC);
static_assert(ca::elf::symbol_bindings::local == STB_LOCAL);
static_assert(ca::elf::symbol_bindings::global == STB_GLOBAL);
static_assert(ca::elf::symbol_bindings::weak == STB_WEAK);
static_assert(ca::elf::symbol_bindings::gnu_unique == STB_GNU_UNIQUE);
static_assert(ca::elf::symbol_visibilities::normal == STV_DEFAULT);
static_assert(ca::elf::symbol_visibilities::internal == STV_INTERNAL);
static_assert(ca::elf::symbol_visibilities::hidden == STV_HIDDEN);
static_assert(ca::elf::symbol_visibilities::nonpreemptible == STV_PROTECTED);

static_assert(ca::elf::dynamic_tags::null == DT_NULL);
static_assert(ca::elf::dynamic_tags::needed == DT_NEEDED);
static_assert(ca::elf::dynamic_tags::plt_relocations_size == DT_PLTRELSZ);
static_assert(ca::elf::dynamic_tags::plt_got == DT_PLTGOT);
static_assert(ca::elf::dynamic_tags::hash == DT_HASH);
static_assert(ca::elf::dynamic_tags::strings == DT_STRTAB);
static_assert(ca::elf::dynamic_tags::symbols == DT_SYMTAB);
static_assert(ca::elf::dynamic_tags::rela == DT_RELA);
static_assert(ca::elf::dynamic_tags::rela_size == DT_RELASZ);
static_assert(ca::elf::dynamic_tags::rela_entry == DT_RELAENT);
static_assert(ca::elf::dynamic_tags::strings_size == DT_STRSZ);
static_assert(ca::elf::dynamic_tags::symbol_entry == DT_SYMENT);
static_assert(ca::elf::dynamic_tags::init == DT_INIT);
static_assert(ca::elf::dynamic_tags::fini == DT_FINI);
static_assert(ca::elf::dynamic_tags::soname == DT_SONAME);
static_assert(ca::elf::dynamic_tags::rpath == DT_RPATH);
static_assert(ca::elf::dynamic_tags::symbolic == DT_SYMBOLIC);
static_assert(ca::elf::dynamic_tags::rel == DT_REL);
static_assert(ca::elf::dynamic_tags::rel_size == DT_RELSZ);
static_assert(ca::elf::dynamic_tags::rel_entry == DT_RELENT);
static_assert(ca::elf::dynamic_tags::plt_relocation_kind == DT_PLTREL);
static_assert(ca::elf::dynamic_tags::debug == DT_DEBUG);
static_assert(ca::elf::dynamic_tags::text_relocations == DT_TEXTREL);
static_assert(ca::elf::dynamic_tags::plt_relocations == DT_JMPREL);
static_assert(ca::elf::dynamic_tags::bind_now == DT_BIND_NOW);
static_assert(ca::elf::dynamic_tags::init_array == DT_INIT_ARRAY);
static_assert(ca::elf::dynamic_tags::fini_array == DT_FINI_ARRAY);
static_assert(ca::elf::dynamic_tags::init_array_size == DT_INIT_ARRAYSZ);
static_assert(ca::elf::dynamic_tags::fini_array_size == DT_FINI_ARRAYSZ);
static_assert(ca::elf::dynamic_tags::runpath == DT_RUNPATH);
static_assert(ca::elf::dynamic_tags::flags == DT_FLAGS);
static_assert(ca::elf::dynamic_tags::preinit_array == DT_PREINIT_ARRAY);
static_assert(ca::elf::dynamic_tags::preinit_array_size == DT_PREINIT_ARRAYSZ);
static_assert(ca::elf::dynamic_tags::relr_size == DT_RELRSZ);
static_assert(ca::elf::dynamic_tags::relr == DT_RELR);
static_assert(ca::elf::dynamic_tags::relr_entry == DT_RELRENT);
static_assert(ca::elf::dynamic_tags::gnu_hash == DT_GNU_HASH);
static_assert(ca::elf::dynamic_tags::version_symbols == DT_VERSYM);
static_assert(ca::elf::dynamic_tags::rela_count == DT_RELACOUNT);
static_assert(ca::elf::dynamic_tags::flags_1 == DT_FLAGS_1);
static_assert(ca::elf::dynamic_tags::version_definitions == DT_VERDEF);
static_assert(ca::elf::dynamic_tags::version_definition_count == DT_VERDEFNUM);
static_assert(ca::elf::dynamic_tags::version_needs == DT_VERNEED);
static_assert(ca::elf::dynamic_tags::version_need_count == DT_VERNEEDNUM);
static_assert(ca::elf::dynamic_flags::origin == DF_ORIGIN);
static_assert(ca::elf::dynamic_flags::symbolic == DF_SYMBOLIC);
static_assert(ca::elf::dynamic_flags::text_relocations == DF_TEXTREL);
static_assert(ca::elf::dynamic_flags::bind_now == DF_BIND_NOW);
static_assert(ca::elf::dynamic_flags::static_tls == DF_STATIC_TLS);
static_assert(ca::elf::dynamic_flags_1::now == DF_1_NOW);
static_assert(ca::elf::dynamic_flags_1::global == DF_1_GLOBAL);
static_assert(ca::elf::dynamic_flags_1::no_delete == DF_1_NODELETE);
static_assert(ca::elf::dynamic_flags_1::pie == DF_1_PIE);

static_assert(ca::elf::x86_64_relocations::none == R_X86_64_NONE);
static_assert(ca::elf::x86_64_relocations::absolute == R_X86_64_64);
static_assert(ca::elf::x86_64_relocations::copy == R_X86_64_COPY);
static_assert(ca::elf::x86_64_relocations::global_data == R_X86_64_GLOB_DAT);
static_assert(ca::elf::x86_64_relocations::jump_slot == R_X86_64_JUMP_SLOT);
static_assert(ca::elf::x86_64_relocations::relative == R_X86_64_RELATIVE);
static_assert(ca::elf::x86_64_relocations::tls_module == R_X86_64_DTPMOD64);
static_assert(ca::elf::x86_64_relocations::tls_offset == R_X86_64_DTPOFF64);
static_assert(ca::elf::x86_64_relocations::tls_thread_offset == R_X86_64_TPOFF64);
static_assert(ca::elf::x86_64_relocations::tls_descriptor == R_X86_64_TLSDESC);
static_assert(ca::elf::x86_64_relocations::indirect_relative == R_X86_64_IRELATIVE);

static_assert(ca::elf::auxiliary_types::null == AT_NULL);
static_assert(ca::elf::auxiliary_types::executable_fd == AT_EXECFD);
static_assert(ca::elf::auxiliary_types::program_headers == AT_PHDR);
static_assert(ca::elf::auxiliary_types::program_header_size == AT_PHENT);
static_assert(ca::elf::auxiliary_types::program_header_count == AT_PHNUM);
static_assert(ca::elf::auxiliary_types::page_size == AT_PAGESZ);
static_assert(ca::elf::auxiliary_types::interpreter_base == AT_BASE);
static_assert(ca::elf::auxiliary_types::flags == AT_FLAGS);
static_assert(ca::elf::auxiliary_types::entry == AT_ENTRY);
static_assert(ca::elf::auxiliary_types::uid == AT_UID);
static_assert(ca::elf::auxiliary_types::effective_uid == AT_EUID);
static_assert(ca::elf::auxiliary_types::gid == AT_GID);
static_assert(ca::elf::auxiliary_types::effective_gid == AT_EGID);
static_assert(ca::elf::auxiliary_types::platform == AT_PLATFORM);
static_assert(ca::elf::auxiliary_types::hardware_capabilities == AT_HWCAP);
static_assert(ca::elf::auxiliary_types::clock_ticks == AT_CLKTCK);
static_assert(ca::elf::auxiliary_types::secure == AT_SECURE);
static_assert(ca::elf::auxiliary_types::random == AT_RANDOM);
static_assert(ca::elf::auxiliary_types::hardware_capabilities_2 == AT_HWCAP2);
static_assert(ca::elf::auxiliary_types::executable_name == AT_EXECFN);
static_assert(ca::elf::auxiliary_types::vdso == AT_SYSINFO_EHDR);
static_assert(ca::elf::auxiliary_types::minimum_signal_stack == AT_MINSIGSTKSZ);
