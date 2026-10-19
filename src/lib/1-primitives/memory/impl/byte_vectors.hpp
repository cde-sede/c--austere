#ifndef ca_primitives_memory_impl_byte_vectors_hpp
#define ca_primitives_memory_impl_byte_vectors_hpp

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/memory/impl/loose_bytes.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BYTE VECTORS -------------------------------------------------------

#if ca_architecture(x86_64)

	namespace impl {

		/* @DOC Bit i set when byte i of v has its high bit set (pmovmskb): of an equality
		   result, the lanes that compared equal. */
		[[gnu::always_inline]] inline auto byte_mask(const ca::impl::bytes16& v) noexcept -> ca::u32 {
			return static_cast<ca::u32>(__builtin_ia32_pmovmskb128(v));
		}

		/* @DOC Not always_inline: generic templates calling it are compiled without AVX2; the
		   AVX2 variants are flattened, so it is inlined there. The mask is opaque: clang
		   otherwise tests it with vpxor and vptest (3 uops, then the compare again). */
		[[gnu::target("avx2")]] inline auto byte_mask(const ca::impl::bytes32& v) noexcept -> ca::u32 {

			ca::u32 bits = static_cast<ca::u32>(__builtin_ia32_pmovmskb256(v));
			asm("" : "+r"(bits));
			return bits;
		}

		/* @DOC 16 or 32 bytes at p into v, in asm: the compiler sees no access beyond the range
		   the caller owns, which these loads may pass, never beyond p's page (the caller
		   checks). The 16-byte one is legacy SSE (AVX state is clean at entry); the 32-byte
		   one, as byte_mask, by reference (no vector crosses an ABI). */
		[[gnu::always_inline]] inline auto peek(const char* const p, ca::impl::bytes16& v) noexcept -> void {
			asm("movdqu %1, %0" : "=x"(v) : "m"(*reinterpret_cast<const char(*)[16]>(p)));
		}

		[[gnu::target("avx2")]] inline auto peek(const char* const p, ca::impl::bytes32& v) noexcept -> void {
			asm("vmovdqu %1, %0" : "=x"(v) : "m"(*reinterpret_cast<const char(*)[32]>(p)));
		}

		/* @DOC peek at p aligned to the vector size: such a load never crosses a page, so it
		   may cover bytes before and after any range inside it. */
		[[gnu::always_inline]] inline auto peek_aligned(const char* const p, ca::impl::bytes16& v) noexcept -> void {
			asm("movdqa %1, %0" : "=x"(v) : "m"(*reinterpret_cast<const char(*)[16]>(p)));
		}

		[[gnu::target("avx2")]] inline auto peek_aligned(const char* const p, ca::impl::bytes32& v) noexcept -> void {
			asm("vmovdqa %1, %0" : "=x"(v) : "m"(*reinterpret_cast<const char(*)[32]>(p)));
		}

	} // namespace impl

#endif

} // namespace ca

#endif // ca_primitives_memory_impl_byte_vectors_hpp
