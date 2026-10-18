#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/io/isatty.hpp"
#include "3-system/thread/thread_key.hpp"
#include "3-system/thread/thread_key_create.hpp"
#include "3-system/thread/thread_key_set.hpp"
#include "6-resources/io/out.hpp"


static constexpr ca::usz out_capacity = 4096U;
static constexpr ca::u32 key_none     = 0U;
static constexpr ca::u32 key_creating = 1U;
static constexpr ca::u32 key_ready    = 2U;
static constexpr ca::u32 key_failed   = 3U;



// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- OUT ----------------------------------------------------------------

	namespace io {


		namespace impl {

			constinit thread_local ca::io::writer* out_current = nullptr;

			/* @DOC Flushes the exiting thread's out() at exit. */
			struct out_exit_flush final {

					using self = ca::io::impl::out_exit_flush;

					constexpr out_exit_flush(void) noexcept {
					}

					~out_exit_flush(void) noexcept {
						if (ca::io::impl::out_current != nullptr)
							static_cast<void>(ca::io::impl::out_current->flush());
					}

			}; // struct out_exit_flush

		} // namespace impl

	} // namespace io

} // namespace ca


alignas(ca::io::writer) static constinit thread_local ca::byte out_slot[sizeof(ca::io::writer)]{};
static constinit thread_local char out_buffer[out_capacity]{};
static constinit ca::atomic<ca::u32> key_state{key_none}; // file-state: ignore
static constinit ca::usz key_id = 0U; // file-state: ignore
static constinit ca::io::impl::out_exit_flush exit_flush{}; // file-state: ignore


static auto flush_at_thread_exit(void* const writer) noexcept -> void {
	static_cast<void>(static_cast<ca::io::writer*>(writer)->flush());
}

static auto register_thread_exit(ca::io::writer* const writer) noexcept -> void {

	ca::u32 state = key_none;
	if (key_state.compare_exchange_strong(state, key_creating)) {
		auto key = ca::sys::thread_key_create(&flush_at_thread_exit);
		if (key)
			key_id = key->id();
		key_state.store(key ? key_ready : key_failed);
		state = key ? key_ready : key_failed;
	}
	while (state == key_creating)
		state = key_state.load();
	if (state == key_ready)
		static_cast<void>(ca::sys::thread_key_set(ca::sys::thread_key{key_id}, writer));
}

[[nodiscard, gnu::cold, gnu::noinline]] auto ca::io::impl::open_out(void) noexcept -> ca::io::writer& {

	const ca::io::buffering mode = ca::sys::isatty(1) ? ca::io::buffering::line : ca::io::buffering::full;
	ca::io::writer* const writer = ca::construct_at(reinterpret_cast<ca::io::writer*>(out_slot), 1, mode, ca::span<char>{out_buffer, out_capacity});
	register_thread_exit(writer);
	ca::io::impl::out_current = writer;
	return *writer;
}
