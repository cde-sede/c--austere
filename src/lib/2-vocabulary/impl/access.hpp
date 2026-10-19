#ifndef ca_vocabulary_impl_access_hpp
#define ca_vocabulary_impl_access_hpp

#include "1-primitives/debug/recovery.hpp"
#include "1-primitives/debug/source_location.hpp"
#include "1-primitives/debug/trap.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/utility/addressof.hpp"
#include "1-primitives/utility/type_key.hpp"
#include "2-vocabulary/error.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ACCESS -------------------------------------------------------------

	namespace impl {

		[[noreturn, gnu::cold, gnu::noinline]] inline auto value_accessed_without_a_check(void) noexcept -> void {
			ca::trap("value accessed without a check");
		}

		/* @DOC Leaves error on the innermost recovery point, for the context to fail with: a
		   ca::error, or what error() of E returns, as code and trace; any other E offered whole to
		   the context, which moves it out when it cannot copy it. */
		template <typename E>
		inline auto leave_error(const E& error) noexcept -> void {

			ca::impl::recovery* const point = ca::impl::current_recovery;
			if (point == nullptr)
				return;
			if constexpr (ca::is_same<E, ca::error>) {
				 point->_code = error.code();
				point->_trace = error.trace();
			}
			else {
				if constexpr (requires { { error.error() } -> ca::is_same<ca::error>; }) {
					 point->_code = error.error().code();
					point->_trace = error.error().trace();
				}
				point->_accepted = point->_accept(point->_sink, const_cast<E*>(ca::addressof(error)), ca::impl::type_key<E>);
			}
		}

		/* @DOC The stand-in of the value (the error for a result) goes with the trap, see
		   leave_error. */
		template <typename... E>
		[[noreturn, gnu::cold, gnu::noinline]] inline auto value_accessed_while_absent(const E&... error) noexcept -> void {

			(ca::impl::leave_error(error), ...);
			ca::trap("value accessed while absent");
		}

		/* @DOC result::raise on a failure, at where; the error goes with the trap, see
		   leave_error. An owned error (the failure raise(on_error) made) that the context did not
		   take is cleaned up first when it has a cleanup(), since the trap abandons it. */
		template <typename E>
		[[noreturn, gnu::cold, gnu::noinline]] inline auto error_raised(E& error, const ca::source_location where, const bool owned) noexcept -> void {

			ca::impl::recovery* const point = ca::impl::current_recovery;
			if (point != nullptr)
				point->_where = where;
			ca::impl::leave_error(error);
			if constexpr (requires { error.cleanup(); }) {
				if (owned && (point == nullptr || !point->_accepted))
					static_cast<void>(error.cleanup());
			}
			ca::trap("error raised");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto error_accessed_on_a_success(void) noexcept -> void {
			ca::trap("error accessed on a success");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto absence_propagated_while_engaged(void) noexcept -> void {
			ca::trap("absence propagated while engaged");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto index_out_of_bounds(void) noexcept -> void {
			ca::trap("index out of bounds");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto assigned_over_uncleaned_elements(void) noexcept -> void {
			ca::trap("assigned over uncleaned elements");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto assigned_over_an_uncleaned_object(void) noexcept -> void {
			ca::trap("assigned over an uncleaned object");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto object_accessed_through_an_empty_owner(void) noexcept -> void {
			ca::trap("object accessed through an empty owner");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto element_already_linked(void) noexcept -> void {
			ca::trap("element already linked");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto element_not_linked(void) noexcept -> void {
			ca::trap("element not linked");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto element_destroyed_while_linked(void) noexcept -> void {
			ca::trap("element destroyed while linked");
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto key_not_found(void) noexcept -> void {
			ca::trap("key not found");
		}

		[[gnu::error("value accessed without a check")]] auto proven_unchecked_access(void) noexcept -> void;

	} // namespace impl

} // namespace ca

#endif // ca_vocabulary_impl_access_hpp
