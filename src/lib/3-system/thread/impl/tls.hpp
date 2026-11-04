#ifndef ca_system_thread_impl_tls_hpp
#define ca_system_thread_impl_tls_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- TLS ------------------------------------------------------------

		/* @DOC Thread-local storage without libc (x86-64 variant II): each thread's copy of the
		   elf::segment_types::tls image sits right below its thread pointer, at the offsets the linker gave the
		   variables, and the control block above it: the pointer itself at %fs:0, the stack canary
		   at %fs:0x28. _start finds the image and lays out the main thread's area; a thread ca
		   starts gets its own area the same way. The functions run before %fs is set: no
		   thread_local, no stack protector, no call that fails. Linux x86-64 only. */
		namespace impl {

			/* @DOC Bytes of the control block above the thread pointer. */
			inline constexpr ca::usz tls_control_block = 256U;

			/* @DOC The program's TLS image: its initialized bytes, their count, the block's size
			   (p_memsz rounded up to the image's own alignment, as the linker places variables)
			   and the alignment of the thread pointer (the image's, at least 64). */
			struct tls_image final {
					using self = ca::sys::impl::tls_image;
					const char* _data;
					ca::usz _file_size;
					ca::usz _size;
					ca::usz _align;
			}; // struct tls_image

			/* @DOC Bytes of the TLS block of a PT_TLS segment of memory_size bytes aligned to align:
			   rounded up to that alignment, as the linker places variables; the block ends at the
			   thread pointer, so a variable at offset in it is at offset - this from it. */
			[[nodiscard, gnu::always_inline]] constexpr auto tls_block_size(const ca::u64 memory_size, const ca::u64 align) noexcept -> ca::usz {

				const ca::u64 own = align > 1U ? align : 1U;
				return static_cast<ca::usz>((memory_size + own - 1U) & ~(own - 1U));
			}

			/* @DOC The image of the running program, set by _start; empty (_align 0) in a program
			   started by libc. */
			extern constinit ca::sys::impl::tls_image program_tls;

			/* @DOC The image from the program headers the kernel passed (AT_PHDR, AT_PHNUM). */
			[[nodiscard, gnu::no_stack_protector]] auto find_tls_image(const void* program_headers, ca::usz count) noexcept -> ca::sys::impl::tls_image;

			/* @DOC Bytes an area for image needs: the block, the control block, slack to align. */
			[[nodiscard]] constexpr auto tls_area_size(const ca::sys::impl::tls_image& image) noexcept -> ca::usz {
				return image._size + ca::sys::impl::tls_control_block + image._align;
			}

			/* @DOC Lays out image and a control block in area (tls_area_size bytes, zeroed) with
			   canary; returns the thread pointer, for %fs. */
			[[nodiscard, gnu::no_stack_protector]] auto install_tls(const ca::sys::impl::tls_image& image, char* area, ca::u64 canary) noexcept -> ca::u64;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_impl_tls_hpp
