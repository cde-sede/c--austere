#ifndef ca_collections_format_impl_render_hpp
#define ca_collections_format_impl_render_hpp

#include "1-primitives/integers/usz.hpp"
#include "5-collections/format/impl/display_argument.hpp"
#include "5-collections/format/impl/format_piece.hpp"
#include "5-collections/format/impl/format_spec.hpp"
#include "5-collections/format/text_writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RENDER -------------------------------------------------------------

	namespace impl {

		/* @DOC The formatting engine, shared by every print and printf: count + 1 pieces of text,
		   count specs and arguments, all parsed and checked at compile time. */
		auto render(ca::text_writer& out, const char* text, const ca::impl::format_piece* pieces, const ca::impl::format_spec* specs, const ca::impl::display_argument* arguments, ca::usz count) noexcept -> void;

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_render_hpp
