#ifndef ca_vocabulary_io_is_reader_hpp
#define ca_vocabulary_io_is_reader_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- IS READER ------------------------------------------------------

		/* @DOC A byte source. read(buffer) fills up to buffer.size() bytes (not 0) and returns how
		   many: 0 only at end of input. Errors come back per call, nothing sticky. char buffers
		   pass as bytes. */
		template <typename R>
		concept is_reader = requires (R& reader, const ca::span<ca::byte> buffer) {
			{ reader.read(buffer) } -> ca::is_same<ca::result<ca::usz>>;
		};

	} // namespace io

} // namespace ca

#endif // ca_vocabulary_io_is_reader_hpp
