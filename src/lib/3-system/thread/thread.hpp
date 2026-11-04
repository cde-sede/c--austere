#ifndef ca_system_thread_thread_hpp
#define ca_system_thread_thread_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- THREAD ---------------------------------------------------------

		namespace impl {

			struct thread_control;

		} // namespace impl


		/* @DOC A thread on raw clone, without libc: one mapping holds a guard page, its stack, a
		   control block and its TLS area. It runs entry(argument), then its thread_local
		   destructors and key destructors, and leaves the kernel, which clears the control block's
		   word and wakes join. join() waits for that, then unmaps. Only in programs started by
		   libca_runtime: libc owns %fs in the others, where make fails with errc::not_supported,
		   as it does outside linux x86-64. Move-only, the mapping stays put. */
		class thread final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::thread;


				// -- private members ----------------------------------------

				ca::span<ca::byte> _mapping;
				ca::sys::impl::thread_control* _control;


				// -- private lifecycle --------------------------------------

				thread(const ca::span<ca::byte> mapping, ca::sys::impl::thread_control* const control) noexcept
				: _mapping{mapping}, _control{control} {
				}


			public:

				// -- public constants ---------------------------------------

				/* @DOC The stack make gives by default, committed as it is touched. */
				static constexpr ca::usz DEFAULT_STACK_SIZE = ca::usz{8U} * 1024U * 1024U;

				/* @DOC The smallest stack make accepts: what the exit path needs. */
				static constexpr ca::usz MINIMUM_STACK_SIZE = ca::usz{16U} * 1024U;


				// -- public lifecycle ---------------------------------------

				/* @DOC Not live: what moving, join() and cleanup() leave. make() starts one. */
				thread(void) noexcept
				: _mapping{}, _control{nullptr} {
				}

				thread(const self&) = delete;

				thread(self&& other) noexcept
				: _mapping{other._mapping}, _control{ca::exchange(other._control, nullptr)} {
				}

				// Failsafe for a skipped cleanup(): would have to wait, nothing to do.
				// ~thread(void) noexcept;

				/* @DOC Starts entry(argument) in a new thread, which inherits the signal mask, with
				   stack_size bytes of stack (rounded up to pages) above a guard page.
				   errc::invalid_argument for a size below MINIMUM_STACK_SIZE or beyond the address
				   space, errc::not_supported as above, the mmap, mprotect or clone error. */
				[[nodiscard]] static auto make(void (*entry)(void*), void* argument, ca::usz stack_size = DEFAULT_STACK_SIZE) noexcept -> ca::result<self>;


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				/* @DOC Takes other's thread. Traps when this one is live, joining being fallible:
				   cleanup() first. */
				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					if (_control != nullptr) [[unlikely]]
						ca::impl::assigned_over_an_uncleaned_object();
					_mapping = other._mapping;
					_control = ca::exchange(other._control, nullptr);
					return *this;
				}


				// -- accessors ----------------------------------------------

				/* @DOC Whether make() started it and it has not been joined. It may have ended. */
				[[nodiscard]] constexpr auto is_live(void) const noexcept -> bool {
					return _control != nullptr;
				}

				/* @DOC The kernel's id of the thread, 0 once it has ended. A live one only. */
				[[nodiscard]] auto id(void) const noexcept -> ca::i32;


				// -- methods ------------------------------------------------

				/* @DOC Waits for the thread's end, then unmaps its stack: not live after, even on
				   failure. errc::invalid_argument when not live. A thread joining itself never
				   returns. */
				[[nodiscard]] auto join(void) noexcept -> ca::result<void>;

				/* @DOC join(), which one not live succeeds. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void>;

		}; // class thread

	} // namespace sys

} // namespace ca

#endif // ca_system_thread_thread_hpp
