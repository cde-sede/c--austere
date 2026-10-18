#ifndef ca_vocabulary_io_is_stream_hpp
#define ca_vocabulary_io_is_stream_hpp

#include "2-vocabulary/io/is_reader.hpp"
#include "2-vocabulary/io/is_writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- IS STREAM ------------------------------------------------------

		/* @DOC Both a reader and a writer: bytes go in by write, come out by read. Each side keeps
		   its own error rules (per call for read; per call and sticky for write). */
		template <typename S>
		concept is_stream = ca::io::is_reader<S> && ca::io::is_writer<S>;

	} // namespace io

} // namespace ca

#endif // ca_vocabulary_io_is_stream_hpp
