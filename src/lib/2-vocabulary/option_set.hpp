#ifndef ca_vocabulary_option_set_hpp
#define ca_vocabulary_option_set_hpp

#include "1-primitives/traits/conditional.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/utility/declval.hpp"
#include "2-vocabulary/inverted.hpp"
#include "2-vocabulary/is_option.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- OPTION SET ---------------------------------------------------------

	/* @DOC A set of options as a type: `a | b` union, `a & b` intersection, `a & ~b` difference,
	   `~a` complement (De Morgan on complements). Everything resolves at compile time; values are
	   empty tags. Order follows first appearance, duplicates dropped. */
	template <ca::is_option... Os>
	class option_set final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::option_set<Os...>;


		public:

			// -- public constants -------------------------------------------

			/* @DOC Whether option O is in the set. */
			template <typename O>
			static constexpr bool HAS = (ca::is_same<O, Os> || ...);

			/* @DOC Whether the platform supports every option in the set. */
			static constexpr bool SUPPORTED = (Os::SUPPORTED && ...);

	}; // class option_set


	namespace impl {

		template <typename... Ts>
		class option_list final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::option_list<Ts...>;

		}; // class option_list

		template <typename T, typename... Us>
		inline constexpr bool is_listed = (ca::is_same<T, Us> || ...);

		template <typename... Ts, typename T>
		constexpr auto operator+(ca::impl::option_list<Ts...>, ca::impl::option_list<T>) noexcept
		-> ca::conditional<ca::is_void<T> || ca::impl::is_listed<T, Ts...>, ca::impl::option_list<Ts...>, ca::impl::option_list<Ts..., T>> {
			return {};
		}

		template <typename... Ts>
		using collected = decltype((ca::impl::option_list<>{} + ... + ca::impl::option_list<Ts>{}));

		template <typename... Ts>
		auto as_option_set(ca::impl::option_list<Ts...>) noexcept -> ca::option_set<Ts...>;

		template <typename... Ts>
		using option_set_of = decltype(ca::impl::as_option_set(ca::impl::collected<Ts...>{}));

		template <typename... Os>
		using option_domains = ca::impl::collected<ca::conditional<Os::SUPPORTED, typename Os::domain, void>...>;

		template <typename D, typename... Os>
		inline constexpr int option_mask = (0 | ... | (ca::is_same<typename Os::domain, D> && Os::SUPPORTED ? static_cast<int>(Os::VALUE) : 0));

	} // namespace impl


	template <typename... As, typename... Bs>
	constexpr auto operator|(ca::option_set<As...>, ca::option_set<Bs...>) noexcept -> ca::impl::option_set_of<As..., Bs...> {
		return {};
	}

	template <typename... As, typename... Bs>
	constexpr auto operator&(ca::option_set<As...>, ca::option_set<Bs...>) noexcept
	-> ca::impl::option_set_of<ca::conditional<ca::impl::is_listed<As, Bs...>, As, void>...> {
		return {};
	}

	template <typename... As, typename... Bs>
	constexpr auto operator&(ca::option_set<As...>, ca::inverted<ca::option_set<Bs...>>) noexcept
	-> ca::impl::option_set_of<ca::conditional<ca::impl::is_listed<As, Bs...>, void, As>...> {
		return {};
	}

	template <typename S, typename... Bs>
	constexpr auto operator&(ca::inverted<S> left, ca::option_set<Bs...> right) noexcept -> decltype(right & left) {
		return {};
	}

	template <typename... Os>
	constexpr auto operator~(ca::option_set<Os...>) noexcept -> ca::inverted<ca::option_set<Os...>> {
		return {};
	}

	template <typename S>
	constexpr auto operator~(ca::inverted<S>) noexcept -> S {
		return {};
	}

	template <typename A, typename B>
	constexpr auto operator|(ca::inverted<A>, ca::inverted<B>) noexcept -> decltype(~(ca::declval<A>() & ca::declval<B>())) {
		return {};
	}

	template <typename A, typename B>
	constexpr auto operator&(ca::inverted<A>, ca::inverted<B>) noexcept -> decltype(~(ca::declval<A>() | ca::declval<B>())) {
		return {};
	}

} // namespace ca

#endif // ca_vocabulary_option_set_hpp
