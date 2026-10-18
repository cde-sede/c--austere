#ifndef ca_vocabulary_unicode_transcoded_hpp
#define ca_vocabulary_unicode_transcoded_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- TRANSCODED -----------------------------------------------------

		/* @DOC What one transcode call did: code units read from the input and written to the
		   output. read() < the input size when the output was full. */
		class transcoded final {


			private:

				// -- private types ------------------------------------------

				using self = ca::unicode::transcoded;


				// -- private members ----------------------------------------

				ca::usz _read;
				ca::usz _written;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr transcoded(const ca::usz read, const ca::usz written) noexcept
				: _read{read}, _written{written} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto read(void) const noexcept -> ca::usz {
					return _read;
				}

				[[nodiscard]] constexpr auto written(void) const noexcept -> ca::usz {
					return _written;
				}

		}; // class transcoded

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_transcoded_hpp
