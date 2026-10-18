#ifndef ca_collections_format_impl_format_spec_hpp
#define ca_collections_format_impl_format_spec_hpp

#include "1-primitives/integers/i16.hpp"
#include "1-primitives/integers/u16.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORMAT SPEC --------------------------------------------------------

	namespace impl {

		/* @DOC One placeholder, parsed at compile time. align 0: the kind's default (numbers right,
		   text left). type 0: the kind's default. wrap: printf's view of a signed integer as the
		   unsigned of its width (%u %x %o %b). */
		struct format_spec final {

				using self = ca::impl::format_spec;

				ca::u16 _width = 0U;
				ca::i16 _precision = -1;
				char _fill = ' ';
				char _align = '\0';
				char _sign = '\0';
				char _type = '\0';
				bool _alternate = false;
				bool _zero = false;
				bool _wrap = false;

		}; // struct format_spec

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_format_spec_hpp
