#ifndef ca_vocabulary_in_place_t_hpp
#define ca_vocabulary_in_place_t_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IN PLACE T ---------------------------------------------------------

	/* @DOC Tag selecting in-place construction. */
	struct in_place_t final {
			using self = ca::in_place_t;
			constexpr explicit in_place_t(void) noexcept = default;
	}; // struct in_place_t

} // namespace ca

#endif // ca_vocabulary_in_place_t_hpp
