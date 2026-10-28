#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "3-system/thread/impl/tls.hpp"

#include <elf.h>

#if ca_platform(linux) && ca_architecture(x86_64)

extern "C" [[gnu::visibility("hidden")]] const Elf64_Ehdr __ehdr_start; // ns-leak: ignore

static constexpr ca::usz CONTROL_ALIGN = 64U;

namespace ca {


	namespace sys {


		namespace impl {

			constinit ca::sys::impl::tls_image program_tls{nullptr, 0U, 0U, 0U};

		} // namespace impl

	} // namespace sys

} // namespace ca

/* The barrier keeps the compiler from making it a memcpy call, whose variant is picked later. */
[[gnu::no_stack_protector]] static auto copy(char* to, const char* from, ca::usz count) noexcept -> void {

	for (; count != 0U; --count)
		*to++ = *from++;
	asm("" : : "r"(to) : "memory");
}

[[nodiscard, gnu::no_stack_protector]] auto ca::sys::impl::find_tls_image(const void* const program_headers, const ca::usz count) noexcept -> ca::sys::impl::tls_image {

	const Elf64_Phdr* const segments = static_cast<const Elf64_Phdr*>(program_headers);
	ca::u64 bias = 0U;
	bool placed = false;
	const Elf64_Phdr* tls = nullptr;
	for (ca::usz i = 0U; i < count; ++i) {
		if (segments[i].p_type == PT_PHDR) {
			  bias = reinterpret_cast<ca::u64>(segments) - segments[i].p_vaddr;
			placed = true;
		} else if (segments[i].p_type == PT_TLS) {
			tls = segments + i;
		}
	}
	for (ca::usz i = 0U; i < count && !placed; ++i) {
		if (segments[i].p_type == PT_LOAD && segments[i].p_offset == 0U) {
			  bias = reinterpret_cast<ca::u64>(segments) - __ehdr_start.e_phoff - segments[i].p_vaddr;
			placed = true;
		}
	}
	if (tls == nullptr)
		return ca::sys::impl::tls_image{nullptr, 0U, 0U, CONTROL_ALIGN};
	const ca::usz own = tls->p_align > 1U ? tls->p_align : 1U;
	return ca::sys::impl::tls_image{
		reinterpret_cast<const char*>(bias + tls->p_vaddr),
		tls->p_filesz,
		(tls->p_memsz + own - 1U) & ~(own - 1U),
		own > CONTROL_ALIGN ? own : CONTROL_ALIGN,
	};
}

[[nodiscard, gnu::no_stack_protector]] auto ca::sys::impl::install_tls(const ca::sys::impl::tls_image& image, char* const area, const ca::u64 canary) noexcept -> ca::u64 {

	const ca::u64 pointer = (reinterpret_cast<ca::u64>(area) + image._size + image._align - 1U) & ~static_cast<ca::u64>(image._align - 1U);
	char* const block = reinterpret_cast<char*>(pointer) - image._size;
	if (image._file_size != 0U)
		copy(block, image._data, image._file_size);
	reinterpret_cast<ca::u64*>(pointer)[0] = pointer;
	reinterpret_cast<ca::u64*>(pointer)[5] = canary;
	return pointer;
}

#endif
