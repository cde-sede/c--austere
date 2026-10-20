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

		/* @DOC Puts error on the trap, for the context taking it to fail with: a ca::error, or what
		   error() of E returns, as code and trace; any other E offered whole, which the context
		   moves out when it cannot copy it. */
		template <typename E>
		inline auto offer_error(ca::impl::trap_info& info, const E& error) noexcept -> void {

			if constexpr (ca::is_same<E, ca::error>) {
				 info._code = error.code();
				info._trace = error.trace();
			}
			else {
				if constexpr (requires { { error.error() } -> ca::is_same<ca::error>; }) {
					 info._code = error.error().code();
					info._trace = error.error().trace();
				}
				info._payload = const_cast<E*>(ca::addressof(error));
				   info._type = ca::impl::type_key<E>;
			}
		}

		template <typename E>
		auto discard_error(void* const error) noexcept -> void {
			static_cast<void>(static_cast<E*>(error)->cleanup());
		}

		/* @DOC The stand-in of the value (the error for a result) goes with the trap, see
		   offer_error. */
		template <typename... E>
		[[noreturn, gnu::cold, gnu::noinline]] inline auto value_accessed_while_absent(const E&... error) noexcept -> void {

			ca::impl::trap_info info{};
			info._reason = "value accessed while absent";
			(ca::impl::offer_error(info, error), ...);
			ca::impl::stop(info);
		}

		/* @DOC result::raise on a failure, at where; the error goes with the trap, see
		   offer_error. An owned error (the failure raise(on_error) made) that no context takes is
		   cleaned up first when it has a cleanup(), since the trap abandons it. */
		template <typename E>
		[[noreturn, gnu::cold, gnu::noinline]] inline auto error_raised(E& error, const ca::source_location where, const bool owned) noexcept -> void {

			ca::impl::trap_info info{};
			info._reason = "error raised";
			 info._where = where;
			ca::impl::offer_error(info, error);
			if constexpr (requires { error.cleanup(); }) {
				if (owned)
					info._discard = &ca::impl::discard_error<E>;
			}
			ca::impl::stop(info);
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
