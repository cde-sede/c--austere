#ifndef ca_vocabulary_impl_access_hpp
#define ca_vocabulary_impl_access_hpp

#include "1-primitives/debug/trap.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ACCESS -------------------------------------------------------------

	namespace impl {

		[[noreturn, gnu::cold, gnu::noinline]] inline auto value_accessed_without_a_check(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto value_accessed_while_absent(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto error_accessed_on_a_success(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto absence_propagated_while_engaged(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto index_out_of_bounds(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto assigned_over_uncleaned_elements(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto assigned_over_an_uncleaned_object(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto object_accessed_through_an_empty_owner(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto element_already_linked(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto element_not_linked(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto element_destroyed_while_linked(void) noexcept -> void {
			ca::trap();
		}

		[[noreturn, gnu::cold, gnu::noinline]] inline auto key_not_found(void) noexcept -> void {
			ca::trap();
		}

		[[gnu::error("value accessed without a check")]] auto proven_unchecked_access(void) noexcept -> void;

	} // namespace impl

} // namespace ca

#endif // ca_vocabulary_impl_access_hpp
