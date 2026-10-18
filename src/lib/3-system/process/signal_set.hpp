#ifndef ca_system_process_signal_set_hpp
#define ca_system_process_signal_set_hpp

#include "1-primitives/debug/trap.hpp"
#include "1-primitives/integers/u64.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- SIGNAL SET -----------------------------------------------------

		namespace impl {

			[[noreturn, gnu::cold, gnu::noinline]] inline auto signal_number_out_of_range(void) noexcept -> void {
				ca::trap();
			}

		} // namespace impl


		/* @DOC A set of signal numbers, 1..64, for sigprocmask and sigwait. add and remove trap outside
		   that range; contains answers false. */
		class signal_set final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::signal_set;


				// -- private members ----------------------------------------

				ca::u64 _bits;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr signal_set(void) noexcept
				: _bits{0U} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto contains(const int signal) const noexcept -> bool {
					return signal >= 1 && signal <= 64 && (_bits & self::_bit(signal)) != 0U;
				}

				[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
					return _bits == 0U;
				}

				[[nodiscard]] constexpr auto operator==(const self&) const noexcept -> bool = default;


				// -- modifiers ----------------------------------------------

				constexpr auto add(const int signal) noexcept -> self& {

					_bits |= self::_bit(signal);
					return *this;
				}

				constexpr auto remove(const int signal) noexcept -> self& {

					_bits &= ~self::_bit(signal);
					return *this;
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] static constexpr auto _bit(const int signal) noexcept -> ca::u64 {

					if (signal < 1 || signal > 64) [[unlikely]]
						ca::sys::impl::signal_number_out_of_range();
					return ca::u64{1U} << (signal - 1);
				}

		}; // class signal_set

	} // namespace sys

} // namespace ca

#endif // ca_system_process_signal_set_hpp
