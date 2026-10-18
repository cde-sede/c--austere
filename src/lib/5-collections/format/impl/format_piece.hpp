#ifndef ca_collections_format_impl_format_piece_hpp
#define ca_collections_format_impl_format_piece_hpp

#include "1-primitives/integers/u32.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORMAT PIECE -------------------------------------------------------

	namespace impl {

		/* @DOC Literal text before a placeholder (or after the last): size chars at offset in the
		   format. escape '{': holds {{ or }} to print once; '%': holds %%; 0: copied as is. */
		struct format_piece final {

				using self = ca::impl::format_piece;

				ca::u32 _offset = 0U;
				ca::u32 _size = 0U;
				char _escape = '\0';

		}; // struct format_piece

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_format_piece_hpp
