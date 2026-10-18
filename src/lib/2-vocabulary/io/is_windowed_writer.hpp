#ifndef ca_vocabulary_io_is_windowed_writer_hpp
#define ca_vocabulary_io_is_windowed_writer_hpp

#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/io/is_writer.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- IS WINDOWED WRITER ---------------------------------------------

		/* @DOC A writer that lends its own buffer, so formatting writes in place, without a copy.
		   One message: window() lends free space, a span of chars (empty: none to lend); when the
		   writer fills it, drain(at) takes [window start, at) and lends fresh space, never empty;
		   on failure the message is over, rolled back as far as nothing of it left the writer, and
		   settle is not called. settle(at) ends the message at at and applies the buffering
		   policy; it fails when writing out does. Errors are also recorded, sticky. */
		template <typename S>
		concept is_windowed_writer = ca::io::is_writer<S> && requires (S& writer, char* const at) {
			{ writer.window() } -> ca::is_same<ca::span<char>>;
			{ writer.drain(at) } -> ca::is_same<ca::result<ca::span<char>>>;
			{ writer.settle(at) } -> ca::is_same<ca::result<void>>;
		};

	} // namespace io

} // namespace ca

#endif // ca_vocabulary_io_is_windowed_writer_hpp
