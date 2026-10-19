#ifndef ca_vocabulary_uncertain_hpp
#define ca_vocabulary_uncertain_hpp

#include "1-primitives/config/debug.hpp"
#include "1-primitives/debug/is_constant_folded.hpp"
#include "1-primitives/debug/unreachable.hpp"
#include "1-primitives/traits/add_pointer.hpp"
#include "1-primitives/traits/is_constant_evaluated.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/utility/addressof.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/impl/unit.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/realized.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNCERTAIN ----------------------------------------------------------

	/* @DOC CRTP base of containers that may lack their value (optional, result). A container starts
	   unchecked. An observer (holds, bool, ok, has_value) checks it and makes it a realized value;
	   any change of contents unchecks it. In debug builds (ca_debug) unchecked access fails: at
	   compile time when the optimizer proves it (-O1 and up) or in constant evaluation, otherwise
	   by a trap. Release builds keep no flag. Accessing an absent value traps in every build;
	   inside a context (ca::context_manager) that fails the context, with the error when it
	   is a ca::error.

	   D derives uncertain<D, T>, befriends it, and privately provides: _realizable() const -> bool;
	   _realized_ref() &, const&, && -> realized<T>&, const&, &&; _absent(F& f) -> f(what stands in
	   for the value). Observers report through _observe(bool); every change of contents calls
	   _unobserve(). */
	template <typename D, typename T> // final: ignore
	class uncertain {


		private:

			// -- private types ----------------------------------------------

			using self = ca::uncertain<D, T>;


			// -- private constants ------------------------------------------

			static constexpr bool _has_value_access = !ca::is_same<T, ca::impl::unit>;


			// -- private members --------------------------------------------

#if ca_debug
			mutable bool _checked{false};
#endif


		public:

			// -- accessors --------------------------------------------------

			/* @DOC Whether the value is there. Counts as a check. */
			[[nodiscard]] constexpr auto holds(void) const noexcept -> bool {
				return this->_observe(this->_self()._realizable());
			}

			/* @DOC A checked container is a realized value. */
			constexpr operator ca::realized<T>&(void) & noexcept requires self::_has_value_access {

				this->_expect_value();
				return this->_self()._realized_ref();
			}

			constexpr operator const ca::realized<T>&(void) const& noexcept requires self::_has_value_access {

				this->_expect_value();
				return this->_self()._realized_ref();
			}

			constexpr operator ca::realized<T>&&(void) && noexcept requires self::_has_value_access {

				this->_expect_value();
				return ca::move(*this)._self()._realized_ref();
			}

			[[nodiscard]] constexpr auto operator->(void) noexcept -> ca::add_pointer<T> requires self::_has_value_access {

				this->_expect_value();
				return this->_self()._realized_ref().operator->();
			}

			[[nodiscard]] constexpr auto operator->(void) const noexcept -> ca::add_pointer<const T>
			requires self::_has_value_access {

				this->_expect_value();
				return this->_self()._realized_ref().operator->();
			}

			/* @DOC The value. Needs a check (debug); traps when absent. */
			[[nodiscard]] constexpr auto operator*(void) & noexcept -> T& requires self::_has_value_access {

				this->_expect_value();
				return *this->_self()._realized_ref();
			}

			[[nodiscard]] constexpr auto operator*(void) const& noexcept -> const T& requires self::_has_value_access {

				this->_expect_value();
				return *this->_self()._realized_ref();
			}

			[[nodiscard]] constexpr auto operator*(void) && noexcept -> T&& requires self::_has_value_access {

				this->_expect_value();
				return *ca::move(*this)._self()._realized_ref();
			}

			/* @DOC The value, as `*`. */
			[[nodiscard]] constexpr auto value(void) & noexcept -> T& requires self::_has_value_access {

				this->_expect_value();
				return *this->_self()._realized_ref();
			}

			[[nodiscard]] constexpr auto value(void) const& noexcept -> const T& requires self::_has_value_access {

				this->_expect_value();
				return *this->_self()._realized_ref();
			}

			[[nodiscard]] constexpr auto value(void) && noexcept -> T&& requires self::_has_value_access {

				this->_expect_value();
				return *ca::move(*this)._self()._realized_ref();
			}


			// -- methods ----------------------------------------------------

			/* @DOC on_real(realized value), or on_other(stand-in: nothing for optional, the error
			   for result). No check needed; both return the same type. */
			template <typename R, typename O>
			constexpr auto match(R&& on_real, O&& on_other) & noexcept -> decltype(auto) {
				return self::_match(this->_self(), on_real, on_other);
			}

			template <typename R, typename O>
			constexpr auto match(R&& on_real, O&& on_other) const& noexcept -> decltype(auto) {
				return self::_match(this->_self(), on_real, on_other);
			}

			template <typename R, typename O>
			constexpr auto match(R&& on_real, O&& on_other) && noexcept -> decltype(auto) {
				return self::_match(ca::move(*this)._self(), on_real, on_other);
			}

			/* @DOC The realized value, or one built from fallback. No check needed. */
			template <typename... Args>
			constexpr auto realize_or(Args&&... fallback) const& noexcept -> ca::realized<T> {

				if (this->_self()._realizable())
					return this->_self()._realized_ref();
				return ca::realized<T>(ca::in_place, ca::forward<Args>(fallback)...);
			}

			template <typename... Args>
			constexpr auto realize_or(Args&&... fallback) && noexcept -> ca::realized<T> {

				if (this->_self()._realizable())
					return ca::move(*this)._self()._realized_ref();
				return ca::realized<T>(ca::in_place, ca::forward<Args>(fallback)...);
			}


		protected:

			// -- protected methods ------------------------------------------

			constexpr auto _observe(const bool value) const noexcept -> bool {

#if ca_debug
				_checked = true;
#endif
				return value;
			}

			constexpr auto _unobserve(void) noexcept -> void {
#if ca_debug
				_checked = false;
#endif
			}

			constexpr auto _expect_checked(void) const noexcept -> void {

#if ca_debug
				if (!ca::is_constant_evaluated() && ca::is_constant_folded(_checked) && !_checked)
					ca::impl::proven_unchecked_access();
				if (!_checked)
					ca::impl::value_accessed_without_a_check();
#endif
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _self(void) & noexcept -> D& {
				return static_cast<D&>(*this);
			}

			constexpr auto _self(void) const& noexcept -> const D& {
				return static_cast<const D&>(*this);
			}

			constexpr auto _self(void) && noexcept -> D&& {
				return static_cast<D&&>(*this);
			}

			constexpr auto _expect_value(void) const noexcept -> void {

				this->_expect_checked();
				if (!this->_self()._realizable()) [[unlikely]] {
					auto absent = [](const auto&... stand_in) noexcept -> void {
						ca::impl::value_accessed_while_absent(stand_in...);
					};
					this->_self()._absent(absent);
					ca::unreachable();
				}
			}

			template <typename S, typename R, typename O>
			static constexpr auto _match(S&& s, R& on_real, O& on_other) noexcept -> decltype(auto) {

				if (!s._realizable())
					return ca::forward<S>(s)._absent(on_other);
				if constexpr (ca::is_same<T, ca::impl::unit>)
					return on_real();
				else
					return on_real(ca::forward<S>(s)._realized_ref());
			}


			// -- assertions -------------------------------------------------

			static_assert(!ca::is_void<T>, "ca::uncertain: value type must be non-void");

	}; // class uncertain

} // namespace ca

#endif // ca_vocabulary_uncertain_hpp
