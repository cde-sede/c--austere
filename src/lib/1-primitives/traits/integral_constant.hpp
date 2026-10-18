#ifndef ca_primitives_traits_integral_constant_hpp
#define ca_primitives_traits_integral_constant_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- INTEGRAL CONSTANT --------------------------------------------------

	/* @DOC std::integral_constant. Not final: traits derive from it. */
	template <typename T, T V> // final: ignore
	struct integral_constant {


		private:

			// -- private types ----------------------------------------------

			using self = ca::integral_constant<T, V>;


		public:

			// -- public types -----------------------------------------------

			using value_type = T;
			using type       = self;


			// -- public constants -------------------------------------------

			static constexpr T value = V;


			// -- accessors --------------------------------------------------

			constexpr operator value_type(void) const noexcept {
				return value;
			}

			constexpr auto operator()(void) const noexcept -> value_type {
				return value;
			}

	}; // struct integral_constant

} // namespace ca

#endif // ca_primitives_traits_integral_constant_hpp
