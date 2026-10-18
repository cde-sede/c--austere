#ifndef ca_lifecycle_make_in_hpp
#define ca_lifecycle_make_in_hpp

#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/make.hpp"
#include "4-lifecycle/memory/allocator_of.hpp"
#include "4-lifecycle/memory/as_allocator.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_allocator_aware.hpp"
#include "4-lifecycle/memory/is_allocator_source.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MAKE IN ------------------------------------------------------------

	namespace impl {

		template <typename T, typename R>
		using rebound = typename T::template rebind<ca::allocator_of<R>>;

		template <typename U, typename A, typename... Args>
		concept has_make_in = requires(const A& allocator, Args&&... args) {
			{ U::make_in(allocator, ca::forward<Args>(args)...) } -> ca::is_same<ca::result<U>>;
		};

	} // namespace impl


	/* @DOC Builds the allocator-aware T over source's allocator: T rebound to allocator_of<R>,
	   source an allocator (ca::heap{}) or a resource (an arena) whose allocator() the result
	   keeps, so that everything the object allocates comes from there. Through U::make_in(
	   allocator, args...) when U has it, else as make<U>(allocator, args...).
	   make_in<unique<File>>(arena, ...) puts a File in the arena; make_in<dynamic_array<int>>(
	   arena) gives an empty dynamic_array<int, arena_allocator>. */
	template <typename T, typename R, typename... Args>
	requires ca::is_allocator_aware<T> && ca::is_allocator_source<R>
	[[nodiscard]] auto make_in(R&& source, Args&&... args) noexcept -> ca::result<ca::impl::rebound<T, R>> {

		using rebound = ca::impl::rebound<T, R>;
		const ca::allocator_of<R> allocator = ca::as_allocator(ca::forward<R>(source));
		if constexpr (ca::impl::has_make_in<rebound, ca::allocator_of<R>, Args...>)
			return rebound::make_in(allocator, ca::forward<Args>(args)...);
		else
			return ca::make<rebound>(allocator, ca::forward<Args>(args)...);
	}

} // namespace ca

#endif // ca_lifecycle_make_in_hpp
