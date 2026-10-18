#ifndef ca_vocabulary_nullopt_t_hpp
#define ca_vocabulary_nullopt_t_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- NULLOPT T ----------------------------------------------------------

	/* @DOC Type of nullopt. */
	struct nullopt_t final {
			using self = ca::nullopt_t;
			constexpr explicit nullopt_t(const int) noexcept {
			}
	}; // struct nullopt_t

} // namespace ca

#endif // ca_vocabulary_nullopt_t_hpp
