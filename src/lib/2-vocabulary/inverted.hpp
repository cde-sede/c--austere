#ifndef ca_vocabulary_inverted_hpp
#define ca_vocabulary_inverted_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- INVERTED -----------------------------------------------------------

	/* @DOC Complement of an option_set, written ~set. Means "turn these off" (d &= ~set) or, on
	   the right of &, set difference. No runtime state. */
	template <typename Set>
	class inverted final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::inverted<Set>;

	}; // class inverted

} // namespace ca

#endif // ca_vocabulary_inverted_hpp
