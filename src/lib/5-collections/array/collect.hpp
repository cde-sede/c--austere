#ifndef ca_collections_array_collect_hpp
#define ca_collections_array_collect_hpp

#include "1-primitives/traits/remove_cvref.hpp"
#include "1-primitives/utility/declval.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- COLLECT ------------------------------------------------------------

	namespace impl {

		/* @DOC Element type of range R; a struct, so gcc does not mangle the builtin trait into
		   the signature. */
		template <typename R>
		struct range_element final {

				using self = ca::impl::range_element<R>;

				using type = ca::remove_cvref<decltype(*ca::declval<const R&>().begin())>;

		}; // struct range_element

	} // namespace impl


	/* @DOC The elements of a range in a new dynamic_array, in order: Python's list(range).
	   `auto fields = ca::collect(line.split(","));` gives a dynamic_array<sv::str>. With an
	   allocator first, the array draws from it. */
	template <ca::is_allocator A, typename R>
	[[nodiscard]] auto collect(const A& allocator, const R& range) noexcept -> ca::result<ca::dynamic_array<typename ca::impl::range_element<R>::type, A>> {

		ca::dynamic_array<typename ca::impl::range_element<R>::type, A> out{allocator};
		for (auto&& element : range) {
			auto added = out.append(element);
			if (!added) [[unlikely]] {
				static_cast<void>(out.cleanup());
				return added.propagate();
			}
		}
		return out;
	}

	template <typename R>
	[[nodiscard]] auto collect(const R& range) noexcept -> ca::result<ca::dynamic_array<typename ca::impl::range_element<R>::type>> {
		return ca::collect(ca::heap{}, range);
	}

} // namespace ca

#endif // ca_collections_array_collect_hpp
