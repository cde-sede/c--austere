#ifndef ca_vocabulary_is_contiguous_hpp
#define ca_vocabulary_is_contiguous_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_convertible.hpp"
#include "1-primitives/traits/is_pointer.hpp"
#include "1-primitives/traits/remove_pointer.hpp"
#include "1-primitives/utility/declval.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS CONTIGUOUS ------------------------------------------------------

	/* @DOC R keeps its elements side by side: data() points to the first, size() counts them.
	   What spans and slices are made from: dynamic_array, ca::slice, strings, views. */
	template <typename R>
	concept is_contiguous = requires (R& range) {
		{ range.data() } -> ca::is_pointer;
		{ range.size() } -> ca::is_convertible<ca::usz>;
	};


	namespace impl {

		/* @DOC Element type of a contiguous R, const when R only gives const access. */
		template <ca::is_contiguous R>
		using element_of = ca::remove_pointer<decltype(ca::declval<R&>().data())>;

	} // namespace impl

} // namespace ca

#endif // ca_vocabulary_is_contiguous_hpp
