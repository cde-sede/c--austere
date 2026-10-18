#ifndef ca_vocabulary_result_hpp
#define ca_vocabulary_result_hpp

#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_default_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_nothrow_move_constructible.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/addressof.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/impl/unit.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/realized.hpp"
#include "2-vocabulary/uncertain.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RESULT -------------------------------------------------------------

	template <typename T, typename E = ca::error>
	class result;


	namespace impl {

		template <typename T>
		inline constexpr bool is_result = false;

		template <typename T, typename E>
		inline constexpr bool is_result<ca::result<T, E>> = true;

		template <typename T>
		inline constexpr bool is_failure = false;

		template <typename E>
		inline constexpr bool is_failure<ca::failure<E>> = true;

		template <typename U>
		inline constexpr bool is_result_special_arg = ca::impl::is_result<U> || ca::impl::is_failure<U>
			|| ca::is_same<U, ca::in_place_t>;

		template <typename T>
		using result_value = ca::conditional<ca::is_void<T>, ca::impl::unit, T>;

	} // namespace impl


	/* @DOC A T, or an error E. Check it (ok, bool) before access, or use match, realize_or,
	   value_or. result<void, E>: success or error. Discarding one is a warning. */
	template <typename T, typename E>
	class [[nodiscard]] result final : public ca::uncertain<ca::result<T, E>, ca::impl::result_value<T>> {


		private:

			// -- friends ----------------------------------------------------

			template <typename, typename>
			friend class ca::result;

			friend class ca::uncertain<ca::result<T, E>, ca::impl::result_value<T>>;


			// -- private types ----------------------------------------------

			using self          = ca::result<T, E>;
			using value_storage = ca::impl::result_value<T>;


			// -- private constants ------------------------------------------

			static constexpr bool _copyable        = ca::is_copy_constructible<value_storage> && ca::is_copy_constructible<E>;
			static constexpr bool _movable         = ca::is_move_constructible<value_storage> && ca::is_move_constructible<E>;
			static constexpr bool _nothrow_movable = ca::is_nothrow_move_constructible<value_storage>
				&& ca::is_nothrow_move_constructible<E>;


		public:

			// -- public types -----------------------------------------------

			using value_type = T;
			using error_type = E;


		private:

			// -- private members --------------------------------------------

			union {
					ca::realized<value_storage> _value;
					ca::realized<E> _error;
			};

			bool _ok;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr result(void) noexcept requires ca::is_default_constructible<value_storage>
			: _value{ca::in_place}, _ok{true} {
			}

			template <typename U = value_storage>
			requires (!ca::impl::is_result_special_arg<ca::remove_cvref<U>> && ca::is_constructible<value_storage, U>)
			constexpr explicit(!ca::is_convertible<U, value_storage>) result(U&& value) noexcept
			: _value{ca::in_place, ca::forward<U>(value)}, _ok{true} {
			}

			template <typename... Args>
			requires ca::is_constructible<value_storage, Args...>
			constexpr explicit result(ca::in_place_t, Args&&... args) noexcept
			: _value{ca::in_place, ca::forward<Args>(args)...}, _ok{true} {
			}

			template <typename G>
			requires ca::is_constructible<E, const G&>
			constexpr result(const ca::failure<G>& f) noexcept
			: _error{ca::in_place, f.error()}, _ok{false} {
			}

			template <typename G>
			requires ca::is_constructible<E, G&&>
			constexpr result(ca::failure<G>&& f) noexcept
			: _error{ca::in_place, ca::move(f).error()}, _ok{false} {
			}

			result(const self& other) noexcept requires self::_copyable
			: _ok{other._ok} {

				if (_ok)
					this->_construct_value(*other._value);
				else
					this->_construct_error(*other._error);
			}

			result(self&& other) noexcept(self::_nothrow_movable) requires self::_movable
			: _ok{other._ok} {

				if (_ok)
					this->_construct_value(*ca::move(other._value));
				else
					this->_construct_error(*ca::move(other._error));
			}

			constexpr ~result(void) noexcept {
				this->_destroy();
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self& other) noexcept -> self& requires self::_copyable {

				if (this != &other)
					this->_assign(other);
				return *this;
			}

			auto operator=(self&& other) noexcept(self::_nothrow_movable) -> self& requires self::_movable {

				if (this != &other)
					this->_assign(ca::move(other));
				return *this;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto ok(void) const noexcept -> bool {
				return this->_observe(_ok);
			}

			constexpr explicit operator bool(void) const noexcept {
				return this->_observe(_ok);
			}

			/* @DOC The error. Needs a check; traps on success. */
			[[nodiscard]] constexpr auto error(void) & noexcept -> E& {

				this->_expect_error();
				return *_error;
			}

			[[nodiscard]] constexpr auto error(void) const& noexcept -> const E& {

				this->_expect_error();
				return *_error;
			}

			[[nodiscard]] constexpr auto error(void) && noexcept -> E&& {

				this->_expect_error();
				return *ca::move(_error);
			}

			/* @DOC The value, or fallback. No check needed. */
			template <typename U>
			[[nodiscard]] constexpr auto value_or(U&& fallback) const& noexcept -> value_storage requires (!ca::is_void<T>) {
				return _ok ? *_value : static_cast<value_storage>(ca::forward<U>(fallback));
			}

			template <typename U>
			[[nodiscard]] constexpr auto value_or(U&& fallback) && noexcept -> value_storage requires (!ca::is_void<T>) {
				return _ok ? *ca::move(_value) : static_cast<value_storage>(ca::forward<U>(fallback));
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Replaces the contents with a value built from args. */
			template <typename... Args>
			requires ca::is_constructible<value_storage, Args...>
			auto emplace(Args&&... args) noexcept -> ca::realized<value_storage>& {

				this->_unobserve();
				this->_destroy();
				this->_construct_value(ca::forward<Args>(args)...);
				return _value;
			}


			// -- methods ----------------------------------------------------

			/* @DOC The error, convertible to any result<U, E>: `if (!r) return r.propagate();`.
			   Needs a check; traps on success. */
			[[nodiscard]] constexpr auto propagate(void) const& noexcept -> ca::failure<E> {

				this->_expect_error();
				return ca::failure<E>{*_error};
			}

			[[nodiscard]] constexpr auto propagate(void) && noexcept -> ca::failure<E> {

				this->_expect_error();
				return ca::failure<E>{*ca::move(_error)};
			}

			/* @DOC f(value) -> result<U, E>; an error passes through. */
			template <typename F>
			constexpr auto and_then(F&& f) & noexcept -> decltype(auto) {
				return self::_and_then(*this, f);
			}

			template <typename F>
			constexpr auto and_then(F&& f) const& noexcept -> decltype(auto) {
				return self::_and_then(*this, f);
			}

			template <typename F>
			constexpr auto and_then(F&& f) && noexcept -> decltype(auto) {
				return self::_and_then(ca::move(*this), f);
			}

			/* @DOC f(value) -> U, wrapped as result<U, E>. */
			template <typename F>
			constexpr auto transform(F&& f) & noexcept -> decltype(auto) {
				return self::_transform(*this, f);
			}

			template <typename F>
			constexpr auto transform(F&& f) const& noexcept -> decltype(auto) {
				return self::_transform(*this, f);
			}

			template <typename F>
			constexpr auto transform(F&& f) && noexcept -> decltype(auto) {
				return self::_transform(ca::move(*this), f);
			}

			/* @DOC f(error) -> result<T, G>; a value passes through. */
			template <typename F>
			constexpr auto or_else(F&& f) & noexcept -> decltype(auto) {
				return self::_or_else(*this, f);
			}

			template <typename F>
			constexpr auto or_else(F&& f) const& noexcept -> decltype(auto) {
				return self::_or_else(*this, f);
			}

			template <typename F>
			constexpr auto or_else(F&& f) && noexcept -> decltype(auto) {
				return self::_or_else(ca::move(*this), f);
			}

			/* @DOC f(error) -> result<T, E>, called only when the error equals match; a value and other
			   errors pass through:
			   `open(path).except(ca::errc::no_entry, [](ca::error) { return ca::result<int>{-1}; })`. */
			template <typename G, typename F>
			requires requires (const E& error, const G& match) { { error == match } -> ca::is_convertible<bool>; }
			constexpr auto except(const G& match, F&& f) & noexcept -> self {
				return self::_except(*this, match, f);
			}

			template <typename G, typename F>
			requires requires (const E& error, const G& match) { { error == match } -> ca::is_convertible<bool>; }
			constexpr auto except(const G& match, F&& f) const& noexcept -> self {
				return self::_except(*this, match, f);
			}

			template <typename G, typename F>
			requires requires (const E& error, const G& match) { { error == match } -> ca::is_convertible<bool>; }
			constexpr auto except(const G& match, F&& f) && noexcept -> self {
				return self::_except(ca::move(*this), match, f);
			}

			/* @DOC f(error) -> G, wrapped as result<T, G>. */
			template <typename F>
			constexpr auto transform_error(F&& f) & noexcept -> decltype(auto) {
				return self::_transform_error(*this, f);
			}

			template <typename F>
			constexpr auto transform_error(F&& f) const& noexcept -> decltype(auto) {
				return self::_transform_error(*this, f);
			}

			template <typename F>
			constexpr auto transform_error(F&& f) && noexcept -> decltype(auto) {
				return self::_transform_error(ca::move(*this), f);
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _realizable(void) const noexcept -> bool {
				return _ok;
			}

			constexpr auto _realized_ref(void) & noexcept -> ca::realized<value_storage>& {
				return _value;
			}

			constexpr auto _realized_ref(void) const& noexcept -> const ca::realized<value_storage>& {
				return _value;
			}

			constexpr auto _realized_ref(void) && noexcept -> ca::realized<value_storage>&& {
				return ca::move(_value);
			}

			template <typename F>
			constexpr auto _absent(F& f) & noexcept -> decltype(auto) {
				return f(*_error);
			}

			template <typename F>
			constexpr auto _absent(F& f) const& noexcept -> decltype(auto) {
				return f(*_error);
			}

			template <typename F>
			constexpr auto _absent(F& f) && noexcept -> decltype(auto) {
				return f(*ca::move(_error));
			}

			constexpr auto _expect_error(void) const noexcept -> void {

				this->_expect_checked();
				if (_ok)
					ca::impl::error_accessed_on_a_success();
			}

			template <typename... Args>
			auto _construct_value(Args&&... args) noexcept -> void {

				ca::construct_at(ca::addressof(_value), ca::in_place, ca::forward<Args>(args)...);
				_ok = true;
			}

			template <typename... Args>
			auto _construct_error(Args&&... args) noexcept -> void {

				ca::construct_at(ca::addressof(_error), ca::in_place, ca::forward<Args>(args)...);
				_ok = false;
			}

			constexpr auto _destroy(void) noexcept -> void {

				if (_ok)
					ca::destroy_at(ca::addressof(_value));
				else
					ca::destroy_at(ca::addressof(_error));
			}

			template <typename Other>
			auto _assign(Other&& other) noexcept -> void {

				this->_unobserve();
				if (_ok && other._ok)
					*_value = *ca::forward<Other>(other)._value;
				else if (!_ok && !other._ok)
					*_error = *ca::forward<Other>(other)._error;
				else {
					this->_destroy();
					if (other._ok)
						this->_construct_value(*ca::forward<Other>(other)._value);
					else
						this->_construct_error(*ca::forward<Other>(other)._error);
				}
			}

			template <typename S, typename F>
			static constexpr auto _call_with_value(S&& s, F& f) noexcept -> decltype(auto) {

				if constexpr (ca::is_void<T>)
					return f();
				else
					return f(*ca::forward<S>(s)._value);
			}

			template <typename R, typename S>
			static constexpr auto _carry_value(S&& s) noexcept -> R {

				if constexpr (ca::is_void<T>)
					return R(ca::in_place);
				else
					return R(ca::in_place, *ca::forward<S>(s)._value);
			}

			template <typename S, typename F>
			static constexpr auto _and_then(S&& s, F& f) noexcept -> decltype(auto) {

				using R = ca::remove_cvref<decltype(self::_call_with_value(ca::forward<S>(s), f))>;
				static_assert(ca::impl::is_result<R>, "ca::result::and_then: f must return a ca::result");
				static_assert(ca::is_same<typename R::error_type, E>, "ca::result::and_then: f must keep the error type");
				if (!s._ok)
					return R(ca::failure<E>{*ca::forward<S>(s)._error});
				return R(self::_call_with_value(ca::forward<S>(s), f));
			}

			template <typename S, typename F>
			static constexpr auto _transform(S&& s, F& f) noexcept -> decltype(auto) {

				using U = ca::remove_cvref<decltype(self::_call_with_value(ca::forward<S>(s), f))>;
				using R = ca::result<U, E>;
				if (!s._ok)
					return R(ca::failure<E>{*ca::forward<S>(s)._error});
				if constexpr (ca::is_void<U>) {
					self::_call_with_value(ca::forward<S>(s), f);
					return R();
				}
				else
					return R(ca::in_place, self::_call_with_value(ca::forward<S>(s), f));
			}

			template <typename S, typename F>
			static constexpr auto _or_else(S&& s, F& f) noexcept -> decltype(auto) {

				using R = ca::remove_cvref<decltype(f(*ca::forward<S>(s)._error))>;
				static_assert(ca::impl::is_result<R>, "ca::result::or_else: f must return a ca::result");
				static_assert(ca::is_same<typename R::value_type, T>, "ca::result::or_else: f must keep the value type");
				if (s._ok)
					return self::_carry_value<R>(ca::forward<S>(s));
				return R(f(*ca::forward<S>(s)._error));
			}

			template <typename S, typename G, typename F>
			static constexpr auto _except(S&& s, const G& match, F& f) noexcept -> self {

				using R = ca::remove_cvref<decltype(f(*ca::forward<S>(s)._error))>;
				static_assert(ca::is_same<R, self>, "ca::result::except: f must return the same ca::result");
				if (s._ok)
					return self::_carry_value<self>(ca::forward<S>(s));
				if (!(*s._error == match))
					return self(ca::failure<E>{*ca::forward<S>(s)._error});
				return f(*ca::forward<S>(s)._error);
			}

			template <typename S, typename F>
			static constexpr auto _transform_error(S&& s, F& f) noexcept -> decltype(auto) {

				using G = ca::remove_cvref<decltype(f(*ca::forward<S>(s)._error))>;
				using R = ca::result<T, G>;
				if (s._ok)
					return self::_carry_value<R>(ca::forward<S>(s));
				return R(ca::failure<G>{f(*ca::forward<S>(s)._error)});
			}


			// -- assertions -------------------------------------------------

			static_assert(!ca::is_reference<T>, "ca::result<T, E>: T cannot be a reference");

	}; // class result

} // namespace ca

#endif // ca_vocabulary_result_hpp
