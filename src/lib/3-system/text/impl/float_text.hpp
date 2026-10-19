#ifndef ca_system_text_impl_float_text_hpp
#define ca_system_text_impl_float_text_hpp

#include "1-primitives/integers/i32.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FLOAT TEXT -----------------------------------------------------

		namespace impl {

			/* @DOC Where float text goes: snprintf's contract, characters past capacity - 1
			   counted but dropped. */
			struct float_sink final {
					using self = ca::sys::impl::float_sink;
					char* _out;
					ca::usz _capacity;
					ca::usz _length;
			}; // struct float_sink

			/* @DOC Decimal digits ('0' - '9'), value 0.d1 d2 ... * 10^_point; digits past _count
			   are zeros. */
			struct float_digits final {
					using self = ca::sys::impl::float_digits;
					const char* _digits;
					ca::usz _count;
					ca::i32 _point;
			}; // struct float_digits

			auto put(ca::sys::impl::float_sink& sink, char c) noexcept -> void;

			/* @DOC NUL-terminates; the length written, at most capacity - 1. */
			[[nodiscard]] auto finish(ca::sys::impl::float_sink& sink) noexcept -> ca::usz;

			/* @DOC "inf" or "nan" (upper: "INF", "NAN"), after '-' when negative. */
			auto put_special(ca::sys::impl::float_sink& sink, bool negative, bool infinite, bool upper) noexcept -> void;

			/* @DOC digits as %f, fraction digits after the point, the point printed when there
			   are some or always. Rounding is the caller's. */
			auto put_fixed(ca::sys::impl::float_sink& sink, const ca::sys::impl::float_digits& digits, ca::i64 fraction, bool always_point) noexcept -> void;

			/* @DOC digits as %e: one digit, fraction more after the point (printed when there are
			   some or always), then exponent_letter and the exponent, signed, 2 digits at least. */
			auto put_scientific(ca::sys::impl::float_sink& sink, const ca::sys::impl::float_digits& digits, ca::i64 fraction, bool always_point, char exponent_letter) noexcept -> void;

			/* @DOC How many of fraction digits after the point remain once trailing zeros go (%g
			   without #): digits at index first, first + 1, ... */
			[[nodiscard]] auto trimmed(const ca::sys::impl::float_digits& digits, ca::i64 first, ca::i64 fraction) noexcept -> ca::i64;

		} // namespace impl

	} // namespace sys

} // namespace ca

#endif // ca_system_text_impl_float_text_hpp
