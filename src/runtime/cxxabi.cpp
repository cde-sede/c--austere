/* What C++ code compiles to calls of, from libstdc++ or libc++abi, for programs linked without
   them or libc (see start_x86_64.cpp): guards of function-local statics, the pure and deleted
   virtual traps, and the global operator new and delete on malloc and free.

   A guard is the Itanium ABI's 64-bit word: the compiler tests its first byte; the first 32 bits
   are a futex word (DONE, PENDING, WAITING as libstdc++'s), the next 32 the thread running the
   initializer, so a recursive initialization traps instead of waiting on itself. Without
   exceptions operator new cannot throw: it traps when memory runs out. */

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/syscall/syscalls.hpp"

#if ca_platform(linux) && ca_architecture(x86_64)

namespace std { // ns-leak: ignore

	enum class align_val_t : decltype(sizeof(0)) {}; // ns-leak: ignore

} // namespace std

extern "C" auto malloc(ca::usz size) noexcept -> void*; // ns-leak: ignore
extern "C" auto free(void* block) noexcept -> void; // ns-leak: ignore
extern "C" auto posix_memalign(void** out, ca::usz alignment, ca::usz size) noexcept -> int; // ns-leak: ignore

static constexpr ca::i32 DONE = 1;
static constexpr ca::i32 PENDING = 0x100;
static constexpr ca::i32 WAITING = 0x10000;

static constexpr int FUTEX_WAIT_PRIVATE = 128;
static constexpr int FUTEX_WAKE_PRIVATE = 129;

[[noreturn, gnu::cold, gnu::noinline]] static auto recursive_initialization(void) noexcept -> void {
	__builtin_trap();
}

[[noreturn, gnu::cold, gnu::noinline]] static auto out_of_memory(void) noexcept -> void {
	__builtin_trap();
}

static auto state_of(ca::i64* const guard) noexcept -> ca::i32* {
	return reinterpret_cast<ca::i32*>(guard);
}

static auto owner_of(ca::i64* const guard) noexcept -> ca::i32* {
	return reinterpret_cast<ca::i32*>(guard) + 1;
}

static auto thread_id(void) noexcept -> ca::i32 {

	const auto id = ca::syscall::gettid();
	return id ? static_cast<ca::i32>(*id) : 0;
}

/* Sets state to done, or back to none when the initializer was abandoned, and wakes those
   waiting. */
static auto settle(ca::i64* const guard, const ca::i32 state) noexcept -> void {

	__atomic_store_n(owner_of(guard), 0, __ATOMIC_RELAXED);
	const ca::i32 was = __atomic_exchange_n(state_of(guard), state, __ATOMIC_RELEASE);
	if ((was & WAITING) != 0)
		static_cast<void>(ca::syscall::futex(state_of(guard), FUTEX_WAKE_PRIVATE, 0x7fffffff, 0, 0, 0));
}

static auto allocate(const ca::usz size) noexcept -> void* {

	void* const block = malloc(size);
	if (block == nullptr) [[unlikely]]
		out_of_memory();
	return block;
}

static auto allocate(const ca::usz size, const std::align_val_t alignment) noexcept -> void* { // no-std: ignore

	void* block = nullptr;
	if (posix_memalign(&block, static_cast<ca::usz>(alignment), size) != 0) [[unlikely]]
		out_of_memory();
	return block;
}

extern "C" {

	auto __cxa_guard_acquire(ca::i64* const guard) noexcept -> int { // ns-leak: ignore

		ca::i32* const state = state_of(guard);
		const ca::i32 self = thread_id();
		for (;;) {
			ca::i32 seen = __atomic_load_n(state, __ATOMIC_ACQUIRE);
			if ((seen & DONE) != 0)
				return 0;
			if (seen == 0) {
				if (__atomic_compare_exchange_n(state, &seen, PENDING, false, __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)) {
					__atomic_store_n(owner_of(guard), self, __ATOMIC_RELAXED);
					return 1;
				}
				continue;
			}
			if (__atomic_load_n(owner_of(guard), __ATOMIC_RELAXED) == self) [[unlikely]]
				recursive_initialization();
			if ((seen & WAITING) == 0 && !__atomic_compare_exchange_n(state, &seen, seen | WAITING, false, __ATOMIC_RELAXED, __ATOMIC_RELAXED))
				continue;
			static_cast<void>(ca::syscall::futex(state, FUTEX_WAIT_PRIVATE, seen | WAITING, 0, 0, 0));
		}
	}

	auto __cxa_guard_release(ca::i64* const guard) noexcept -> void { // ns-leak: ignore
		settle(guard, DONE);
	}

	auto __cxa_guard_abort(ca::i64* const guard) noexcept -> void { // ns-leak: ignore
		settle(guard, 0);
	}

	[[noreturn, gnu::cold]] auto __cxa_pure_virtual(void) noexcept -> void { // ns-leak: ignore
		__builtin_trap();
	}

	[[noreturn, gnu::cold]] auto __cxa_deleted_virtual(void) noexcept -> void { // ns-leak: ignore
		__builtin_trap();
	}

} // extern "C"

auto operator new(const ca::usz size) -> void* { // ns-leak, noexcept: ignore
	return allocate(size);
}

auto operator new[](const ca::usz size) -> void* { // ns-leak, noexcept: ignore
	return allocate(size);
}

auto operator new(const ca::usz size, const std::align_val_t alignment) -> void* { // ns-leak, noexcept, no-std: ignore
	return allocate(size, alignment);
}

auto operator new[](const ca::usz size, const std::align_val_t alignment) -> void* { // ns-leak, noexcept, no-std: ignore
	return allocate(size, alignment);
}

auto operator delete(void* const block) noexcept -> void { // ns-leak: ignore
	free(block);
}

auto operator delete[](void* const block) noexcept -> void { // ns-leak: ignore
	free(block);
}

auto operator delete(void* const block, ca::usz) noexcept -> void { // ns-leak: ignore
	free(block);
}

auto operator delete[](void* const block, ca::usz) noexcept -> void { // ns-leak: ignore
	free(block);
}

auto operator delete(void* const block, std::align_val_t) noexcept -> void { // ns-leak, no-std: ignore
	free(block);
}

auto operator delete[](void* const block, std::align_val_t) noexcept -> void { // ns-leak, no-std: ignore
	free(block);
}

auto operator delete(void* const block, ca::usz, std::align_val_t) noexcept -> void { // ns-leak, no-std: ignore
	free(block);
}

auto operator delete[](void* const block, ca::usz, std::align_val_t) noexcept -> void { // ns-leak, no-std: ignore
	free(block);
}

#endif
