#ifndef ca_resources_dl_impl_tls_hpp
#define ca_resources_dl_impl_tls_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DL -----------------------------------------------------------------

	namespace dl {


		// -- TLS ------------------------------------------------------------

		namespace impl {

			/* @DOC Most loaded objects with thread-local storage at once. */
			inline constexpr ca::usz tls_modules = 1024U;

			/* @DOC The argument of __tls_get_addr, as the general and local dynamic models
			   pass it: a module id (from R_X86_64_DTPMOD64) and an offset in its block (from
			   R_X86_64_DTPOFF64). */
			struct tls_index final {

					using self = ca::dl::impl::tls_index;

					ca::u64 _module;
					ca::u64 _offset;

			}; // struct tls_index

			/* @DOC A module id for a TLS image (its initialized bytes, then zeros up to size,
			   aligned to align); errc::no_space when tls_modules are in use. Under the loader's
			   lock. */
			[[nodiscard]] auto tls_register(const ca::byte* image, ca::usz file_size, ca::usz size, ca::usz align) noexcept -> ca::result<ca::u64>;

			/* @DOC Gives module back: blocks other threads hold for it are freed when they next
			   reach the id or exit; the calling thread's now. Under the loader's lock. */
			auto tls_unregister(ca::u64 module) noexcept -> void;

			/* @DOC __tls_get_addr, the address of index's variable in the calling thread: its
			   block for the module allocated and initialized on first use, freed when the
			   thread exits. Realigns the stack, as callers built by old compilers leave it at
			   8 mod 16. Traps when memory runs out. */
			[[gnu::force_align_arg_pointer]] auto tls_get_addr(const ca::dl::impl::tls_index* index) noexcept -> void*;

		} // namespace impl

	} // namespace dl

} // namespace ca

#endif // ca_resources_dl_impl_tls_hpp
