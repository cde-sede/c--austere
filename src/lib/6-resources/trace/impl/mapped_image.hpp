#ifndef ca_resources_trace_impl_mapped_image_hpp
#define ca_resources_trace_impl_mapped_image_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/trace/impl/debug_info.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- MAPPED IMAGE ---------------------------------------------------

		namespace impl {

			/* @DOC An ELF file a symbolizer mapped to read, its sections, and where the code it
			   describes runs: the bias and the addresses it spans. */
			struct mapped_image final {

					using self = ca::trace::impl::mapped_image;

					ca::span<ca::byte> _mapping;
					ca::trace::impl::debug_image _image;
					ca::usz _bias;
					ca::usz _begin;
					ca::usz _end;

			}; // struct mapped_image

		} // namespace impl

	} // namespace trace

} // namespace ca

#endif // ca_resources_trace_impl_mapped_image_hpp
