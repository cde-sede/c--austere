#ifndef ca_primitives_atomic_atomic_hpp
#define ca_primitives_atomic_atomic_hpp

#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/traits/is_integral.hpp"
#include "1-primitives/traits/is_pointer.hpp"
#include "1-primitives/traits/is_same.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ATOMIC -------------------------------------------------------------

	namespace impl {

		consteval auto failure_order(const ca::memory_order order) noexcept -> ca::memory_order {

			if (order == ca::memory_order::acq_rel)
				return ca::memory_order::acquire;
			if (order == ca::memory_order::release)
				return ca::memory_order::relaxed;
			return order;
		}

	} // namespace impl


	/* @DOC Lock-free atomic integer or pointer. Types needing libatomic are rejected, so every
	   operation is async-signal-safe. Orderings are template arguments, seq_cst by default. */
	template <typename T>
	requires ca::is_integral<T> || ca::is_pointer<T>
	class atomic final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::atomic<T>;


			// -- private members --------------------------------------------

			T _value;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr atomic(void) noexcept
			: _value{} {
			}

			constexpr explicit atomic(const T value) noexcept
			: _value{value} {
			}

			atomic(const self&) = delete;


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;


			// -- accessors --------------------------------------------------

			template <ca::memory_order M = ca::memory_order::seq_cst>
			[[nodiscard]] auto load(void) const noexcept -> T {
				return __atomic_load_n(&_value, static_cast<int>(M));
			}


			// -- modifiers --------------------------------------------------

			template <ca::memory_order M = ca::memory_order::seq_cst>
			auto store(const T value) noexcept -> void {
				__atomic_store_n(&_value, value, static_cast<int>(M));
			}

			template <ca::memory_order M = ca::memory_order::seq_cst>
			auto exchange(const T value) noexcept -> T {
				return __atomic_exchange_n(&_value, value, static_cast<int>(M));
			}

			/* @DOC Stores desired if the value equals expected; otherwise expected receives the
			   value. The failure order defaults from the success order. */
			template <ca::memory_order S = ca::memory_order::seq_cst, ca::memory_order F = ca::impl::failure_order(S)>
			[[nodiscard]] auto compare_exchange_weak(T& expected, const T desired) noexcept -> bool {
				return __atomic_compare_exchange_n(&_value, &expected, desired, true, static_cast<int>(S), static_cast<int>(F));
			}

			template <ca::memory_order S = ca::memory_order::seq_cst, ca::memory_order F = ca::impl::failure_order(S)>
			[[nodiscard]] auto compare_exchange_strong(T& expected, const T desired) noexcept -> bool {
				return __atomic_compare_exchange_n(&_value, &expected, desired, false, static_cast<int>(S), static_cast<int>(F));
			}

			/* @DOC Fetch operations: integers other than bool; return the previous value. */
			template <ca::memory_order M = ca::memory_order::seq_cst>
			auto fetch_add(const T value) noexcept -> T requires (ca::is_integral<T> && !ca::is_same<T, bool>) {
				return __atomic_fetch_add(&_value, value, static_cast<int>(M));
			}

			template <ca::memory_order M = ca::memory_order::seq_cst>
			auto fetch_sub(const T value) noexcept -> T requires (ca::is_integral<T> && !ca::is_same<T, bool>) {
				return __atomic_fetch_sub(&_value, value, static_cast<int>(M));
			}

			template <ca::memory_order M = ca::memory_order::seq_cst>
			auto fetch_and(const T value) noexcept -> T requires (ca::is_integral<T> && !ca::is_same<T, bool>) {
				return __atomic_fetch_and(&_value, value, static_cast<int>(M));
			}

			template <ca::memory_order M = ca::memory_order::seq_cst>
			auto fetch_or(const T value) noexcept -> T requires (ca::is_integral<T> && !ca::is_same<T, bool>) {
				return __atomic_fetch_or(&_value, value, static_cast<int>(M));
			}

			template <ca::memory_order M = ca::memory_order::seq_cst>
			auto fetch_xor(const T value) noexcept -> T requires (ca::is_integral<T> && !ca::is_same<T, bool>) {
				return __atomic_fetch_xor(&_value, value, static_cast<int>(M));
			}


			// -- assertions -------------------------------------------------

			static_assert(__atomic_always_lock_free(sizeof(T), 0), "ca::atomic requires a lock-free type");

	}; // class atomic

} // namespace ca

#endif // ca_primitives_atomic_atomic_hpp
