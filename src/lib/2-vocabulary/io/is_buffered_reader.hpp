#ifndef ca_vocabulary_io_is_buffered_reader_hpp
#define ca_vocabulary_io_is_buffered_reader_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/io/is_reader.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- IS BUFFERED READER ---------------------------------------------

		/* @DOC A reader that lends its buffer, so parsing reads in place, without a copy. fill()
		   returns the buffered bytes not yet consumed, reading more first only when there are
		   none; empty at end of input. The view stays valid until the next call on the reader.
		   consume(n) marks the first n of them (at most all) as read. */
		template <typename R>
		concept is_buffered_reader = ca::io::is_reader<R> && requires (R& reader, const ca::usz count) {
			{ reader.fill() } -> ca::is_same<ca::result<ca::sv::str>>;
			reader.consume(count);
		};

	} // namespace io

} // namespace ca

#endif // ca_vocabulary_io_is_buffered_reader_hpp
