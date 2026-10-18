#ifndef ca_vocabulary_optional_hpp
#define ca_vocabulary_optional_hpp

#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/is_assignable.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_copy_assignable.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_assignable.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_nothrow_move_assignable.hpp"
#include "1-primitives/traits/is_nothrow_move_constructible.hpp"
#include "1-primitives/traits/is_reference.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/addressof.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/realized.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/uncertain.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- OPTIONAL -----------------------------------------------------------

	template <typename T>
	class optional;


	namespace impl {

		template <typename T>
		inline constexpr bool is_optional = false;

		template <typename T>
		inline constexpr bool is_optional<ca::optional<T>> = true;

		template <typename U>
		inline constexpr bool is_optional_special_arg = ca::impl::is_optional<U> || ca::is_same<U, ca::nullopt_t>
			|| ca::is_same<U, ca::in_place_t>;

	} // namespace impl


	/* @DOC A T, or nothing. Check it (has_value, bool) before access, or use match, realize_or,
	   value_or. optional<T&> refers to a T, never to a temporary; assignment rebinds. */
	template <typename T>
	class optional final : public ca::uncertain<ca::optional<T>, T> {


		private:

			// -- friends ----------------------------------------------------

			friend class ca::uncertain<ca::optional<T>, T>;


			// -- private types ----------------------------------------------

			using self     = ca::optional<T>;
			using held     = ca::realized<T>;
			using plain    = ca::conditional<ca::is_reference<T>, ca::remove_cvref<T>, T>;


			// -- private constants ------------------------------------------

			static constexpr bool _copy_constructible         = ca::is_copy_constructible<self::held>;
			static constexpr bool _move_constructible         = ca::is_move_constructible<self::held>;
			static constexpr bool _nothrow_move_constructible = ca::is_nothrow_move_constructible<self::held>;
			static constexpr bool _copyable                   = self::_copy_constructible && ca::is_copy_assignable<self::held>;
			static constexpr bool _movable                    = self::_move_constructible && ca::is_move_assignable<self::held>;
			static constexpr bool _nothrow_movable            = self::_nothrow_move_constructible
				&& ca::is_nothrow_move_assignable<self::held>;


		public:

			// -- public types -----------------------------------------------

			using value_type = T;


		private:

			// -- private members --------------------------------------------

			union {
					char _empty;
					ca::realized<T> _value;
			};

			bool _engaged;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr optional(void) noexcept
			: _empty{}, _engaged{false} {
			}

			constexpr optional(ca::nullopt_t) noexcept
			: _empty{}, _engaged{false} {
			}

			template <typename U = T>
			requires (!ca::impl::is_optional_special_arg<ca::remove_cvref<U>> && ca::impl::is_holdable<T, U>)
			constexpr explicit(!ca::is_convertible<U, T>) optional(U&& value) noexcept
			: _value{ca::in_place, ca::forward<U>(value)}, _engaged{true} {
			}

			template <typename... Args>
			requires ca::impl::is_holdable<T, Args...>
			constexpr explicit optional(ca::in_place_t, Args&&... args) noexcept
			: _value{ca::in_place, ca::forward<Args>(args)...}, _engaged{true} {
			}

			optional(const self& other) noexcept requires self::_copy_constructible
			: _empty{}, _engaged{false} {
				if (other._engaged)
					this->_construct(*other._value);
			}

			optional(self&& other) noexcept(self::_nothrow_move_constructible) requires self::_move_constructible
			: _empty{}, _engaged{false} {
				if (other._engaged)
					this->_construct(*ca::move(other._value));
			}

			constexpr ~optional(void) noexcept {
				this->reset();
			}


			// -- assignment operators ---------------------------------------

			auto operator=(ca::nullopt_t) noexcept -> self& {

				this->reset();
				return *this;
			}

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

			template <typename U = T>
			requires (!ca::impl::is_optional_special_arg<ca::remove_cvref<U>> && ca::impl::is_holdable<T, U>
				&& (ca::is_reference<T> || ca::is_assignable<T&, U>))
			auto operator=(U&& value) noexcept -> self& {

				this->_unobserve();
				if constexpr (ca::is_reference<T>)
					this->emplace(ca::forward<U>(value));
				else if (_engaged)
					*_value = ca::forward<U>(value);
				else
					this->_construct(ca::forward<U>(value));
				return *this;
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto has_value(void) const noexcept -> bool {
				return this->_observe(_engaged);
			}

			constexpr explicit operator bool(void) const noexcept {
				return this->_observe(_engaged);
			}

			/* @DOC The value, or fallback. No check needed. optional<T&>: a copy. */
			template <typename U>
			[[nodiscard]] constexpr auto value_or(U&& fallback) const& noexcept -> self::plain {
				return _engaged ? *_value : static_cast<self::plain>(ca::forward<U>(fallback));
			}

			template <typename U>
			[[nodiscard]] constexpr auto value_or(U&& fallback) && noexcept -> self::plain {
				return _engaged ? *ca::move(_value) : static_cast<self::plain>(ca::forward<U>(fallback));
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Replaces the contents with a value built from args. */
			template <typename... Args>
			requires ca::impl::is_holdable<T, Args...>
			auto emplace(Args&&... args) noexcept -> ca::realized<T>& {

				this->reset();
				this->_construct(ca::forward<Args>(args)...);
				return _value;
			}

			/* @DOC Destroys the value, if any. */
			constexpr auto reset(void) noexcept -> void {

				this->_unobserve();
				if (!_engaged)
					return;
				ca::destroy_at(ca::addressof(_value));
				_engaged = false;
			}


			// -- methods ----------------------------------------------------

			/* @DOC nullopt, for any optional: `if (!o) return o.propagate();`. Needs a check; traps
			   when engaged. */
			[[nodiscard]] constexpr auto propagate(void) const noexcept -> ca::nullopt_t {

				this->_expect_checked();
				if (_engaged)
					ca::impl::absence_propagated_while_engaged();
				return ca::nullopt;
			}

			/* @DOC The value as a success, or f as the error. */
			template <typename G>
			[[nodiscard]] constexpr auto or_fail(ca::failure<G> f) const& noexcept -> ca::result<T, G> requires (!ca::is_reference<T>) {

				if (_engaged)
					return ca::result<T, G>(ca::in_place, *_value);
				return ca::result<T, G>(ca::move(f));
			}

			template <typename G>
			[[nodiscard]] constexpr auto or_fail(ca::failure<G> f) && noexcept -> ca::result<T, G> requires (!ca::is_reference<T>) {

				if (_engaged)
					return ca::result<T, G>(ca::in_place, *ca::move(_value));
				return ca::result<T, G>(ca::move(f));
			}

			/* @DOC f(value) -> optional<U>; empty passes through. */
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

			/* @DOC f(value) -> U, wrapped as optional<U>. */
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

			/* @DOC f() -> optional<T>, called only when empty. */
			template <typename F>
			constexpr auto or_else(F&& f) const& noexcept -> self {
				return self::_or_else(*this, f);
			}

			template <typename F>
			constexpr auto or_else(F&& f) && noexcept -> self {
				return self::_or_else(ca::move(*this), f);
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _realizable(void) const noexcept -> bool {
				return _engaged;
			}

			constexpr auto _realized_ref(void) & noexcept -> ca::realized<T>& {
				return _value;
			}

			constexpr auto _realized_ref(void) const& noexcept -> const ca::realized<T>& {
				return _value;
			}

			constexpr auto _realized_ref(void) && noexcept -> ca::realized<T>&& {
				return ca::move(_value);
			}

			template <typename F>
			constexpr auto _absent(F& f) const noexcept -> decltype(auto) {
				return f();
			}

			template <typename... Args>
			auto _construct(Args&&... args) noexcept -> void {

				ca::construct_at(ca::addressof(_value), ca::in_place, ca::forward<Args>(args)...);
				_engaged = true;
			}

			template <typename Other>
			auto _assign(Other&& other) noexcept -> void {

				this->_unobserve();
				if (_engaged && other._engaged)
					_value = ca::forward<Other>(other)._value;
				else if (other._engaged)
					this->_construct(*ca::forward<Other>(other)._value);
				else
					this->reset();
			}

			template <typename S, typename F>
			static constexpr auto _and_then(S&& s, F& f) noexcept -> decltype(auto) {

				using R = ca::remove_cvref<decltype(f(*ca::forward<S>(s)._value))>;
				static_assert(ca::impl::is_optional<R>, "ca::optional::and_then: f must return a ca::optional");
				if (!s._engaged)
					return R();
				return R(f(*ca::forward<S>(s)._value));
			}

			template <typename S, typename F>
			static constexpr auto _transform(S&& s, F& f) noexcept -> decltype(auto) {

				using U = ca::remove_cvref<decltype(f(*ca::forward<S>(s)._value))>;
				static_assert(!ca::is_void<U>, "ca::optional::transform: f must return a value");
				if (!s._engaged)
					return ca::optional<U>();
				return ca::optional<U>(ca::in_place, f(*ca::forward<S>(s)._value));
			}

			template <typename S, typename F>
			static constexpr auto _or_else(S&& s, F& f) noexcept -> self {

				static_assert(ca::is_same<ca::remove_cvref<decltype(f())>, self>,
					"ca::optional::or_else: f must return the same ca::optional type");
				if (s._engaged)
					return self(ca::in_place, *ca::forward<S>(s)._value);
				return f();
			}


			// -- assertions -------------------------------------------------

			static_assert(!ca::is_same<ca::remove_cvref<T>, ca::nullopt_t> && !ca::is_same<ca::remove_cvref<T>, ca::in_place_t>,
				"ca::optional<T>: T cannot be nullopt_t or in_place_t");

	}; // class optional


	template <typename T>
	optional(T) -> optional<T>; // qualify: ignore


	/* @DOC Comparisons need no check: against nullopt, another optional, or a value. */
	template <typename T>
	constexpr auto operator==(const ca::optional<T>& o, ca::nullopt_t) noexcept -> bool {
		return !o;
	}

	template <typename T, typename U>
	constexpr auto operator==(const ca::optional<T>& a, const ca::optional<U>& b) noexcept -> bool {
		return a.match([&](const ca::realized<T>& x) {
			return b.match([&](const ca::realized<U>& y) {
				return static_cast<bool>(*x == *y);
			}, [] {
				return false;
			});
		}, [&] {
			return !b;
		});
	}

	template <typename T, typename U>
	requires (!ca::impl::is_optional<U> && !ca::is_same<U, ca::nullopt_t>)
	constexpr auto operator==(const ca::optional<T>& a, const U& b) noexcept -> bool {
		return a.match([&](const ca::realized<T>& x) {
			return static_cast<bool>(*x == b);
		}, [] {
			return false;
		});
	}

} // namespace ca

#endif // ca_vocabulary_optional_hpp
