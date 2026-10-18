#ifndef ca_collections_map_impl_range_hpp
#define ca_collections_map_impl_range_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {


		// -- RANGE ----------------------------------------------------------

		/* @DOC A begin and an end, for range-for: what keys(), values() and items() return. */
		template <typename I>
		struct range final {

				using self = ca::impl::range<I>;

				I _begin;
				I _end;

				[[nodiscard]] constexpr auto begin(void) const noexcept -> I {
					return _begin;
				}

				[[nodiscard]] constexpr auto end(void) const noexcept -> I {
					return _end;
				}

		}; // struct range

	} // namespace impl

} // namespace ca

#endif // ca_collections_map_impl_range_hpp
