#ifndef ca_vocabulary_unicode_replacement_character_hpp
#define ca_vocabulary_unicode_replacement_character_hpp



// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- REPLACEMENT CHARACTER ------------------------------------------

		/* @DOC U+FFFD, what decoding yields for an invalid sequence and encoding writes for a
		   value that is not a scalar value. */
		inline constexpr char32_t replacement_character = U'\uFFFD';

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_replacement_character_hpp
