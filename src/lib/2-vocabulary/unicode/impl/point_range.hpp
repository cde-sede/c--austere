#ifndef ca_vocabulary_unicode_impl_point_range_hpp
#define ca_vocabulary_unicode_impl_point_range_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNICODE ------------------------------------------------------------

	namespace unicode {


		// -- POINT RANGE ----------------------------------------------------

		namespace impl {

			/* @DOC Code points first to last, both included. */
			struct point_range final {

					using self = ca::unicode::impl::point_range;

					char32_t _first;
					char32_t _last;

			}; // struct point_range

			/* @DOC One step of a caseless class: point and the next member of its cycle. */
			struct caseless_link final {

					using self = ca::unicode::impl::caseless_link;

					char32_t _point;
					char32_t _next;

			}; // struct caseless_link

		} // namespace impl

	} // namespace unicode

} // namespace ca

#endif // ca_vocabulary_unicode_impl_point_range_hpp
