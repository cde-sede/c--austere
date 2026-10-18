#ifndef ca_vocabulary_io_is_writer_hpp
#define ca_vocabulary_io_is_writer_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IO -----------------------------------------------------------------

	namespace io {


		// -- IS WRITER ------------------------------------------------------

		/* @DOC A byte sink print and printf can target. Errors are reported twice: each call
		   returns its own (write(bytes) -> result<void>: stop early when output is impossible),
		   and the writer keeps the first one since the last check, sticky (one check point for
		   deferred or scattered writes). error() peeks at it; clear_error() forgets it once
		   handled, so it is not reported again; flush() writes out what is pending, then returns
		   and clears it. Text passes as bytes: write(ca::sv::str{...}). */
		template <typename S>
		concept is_writer = requires (S& writer, const S& view, const ca::span<const ca::byte> bytes) {
			{ writer.write(bytes) } -> ca::is_same<ca::result<void>>;
			{ writer.flush() } -> ca::is_same<ca::result<void>>;
			{ view.error() } -> ca::is_same<ca::error>;
			writer.clear_error();
		};

	} // namespace io

} // namespace ca

#endif // ca_vocabulary_io_is_writer_hpp
