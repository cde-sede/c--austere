#ifndef ca_lifecycle_unique_hpp
#define ca_lifecycle_unique_hpp

#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"
#include "4-lifecycle/make.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- UNIQUE -------------------------------------------------------------

	/* @DOC Sole owner of one T in memory from A, or of nothing; from make_in (ca::make_in<unique<T>>
	   over any allocator). Move-only; the allocator moves with the object. cleanup() runs T's cleanup() when T is cleanable, then its
	   destructor, then gives the memory back; the destructor is a failsafe doing the same
	   silently. Holding a stateless allocator, as wide as a pointer. */
	template <typename T, ca::is_allocator A = ca::heap>
	class unique final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::unique<T, A>;


		public:

			// -- public types -----------------------------------------------

			using type            = T;
			using allocator_type  = A;
			using pointer         = T*;
			using const_pointer   = const T*;

			/* @DOC The same owner type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::unique<T, B>;


			// -- public constants -------------------------------------------

			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			T*                      _object;
			[[no_unique_address]] A _allocator;


			// -- private lifecycle ------------------------------------------

			constexpr unique(T* const object, const A& allocator) noexcept
			: _object{object}, _allocator{allocator} {
			}


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC Owning nothing. */
			constexpr unique(void) noexcept requires ca::is_stateless_allocator<A>
			: _object{nullptr}, _allocator{} {
			}

			/* @DOC Owning nothing, with allocator for what it adopts later. */
			constexpr explicit unique(const A& allocator) noexcept
			: _object{nullptr}, _allocator{allocator} {
			}

			unique(const self&) = delete;

			constexpr unique(self&& other) noexcept
			: _object{ca::exchange(other._object, nullptr)}, _allocator{other._allocator} {
			}

			// Failsafe for a skipped cleanup(): destroys the object and frees, silently.
			// ~unique(void) noexcept {
			// 	this->_release();
			// }

			/* @DOC Takes ownership of object, built in memory of at least sizeof(T) bytes, aligned for
			   T, from allocator or a copy of it. */
			[[nodiscard]] static constexpr auto adopt(T* const object, const A& allocator) noexcept -> self {
				return self{object, allocator};
			}

			/* @DOC A T built from args as make builds it, in memory from allocator. Fails without
			   building when the allocation does; a failed constructor gives its memory back first. */
			template <typename... Args>
			requires ca::impl::is_makeable<T, Args...>
			[[nodiscard]] static auto make_in(const A& allocator, Args&&... args) noexcept -> ca::result<self> {

				auto block = allocator.allocate(sizeof(T), alignof(T));
				if (!block) [[unlikely]]
					return block.propagate();
				T* const object = static_cast<T*>(block->data());
				if constexpr (ca::impl::has_reporting_constructor<T, Args...>) {
					ca::error error{};
					ca::construct_at(object, error, ca::forward<Args>(args)...);
					if (!error.ok()) [[unlikely]] {
						ca::destroy_at(object);
						allocator.deallocate(block->with_size(sizeof(T)), alignof(T));
						return ca::failure{error};
					}
				}
				else
					ca::construct_at(object, ca::forward<Args>(args)...);
				return self{object, allocator};
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's object and allocator; the previous object is released as by the
			   destructor. Traps when T is cleanable and an object remains: cleanup() first. */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				if constexpr (ca::is_cleanable<T>) {
					if (_object != nullptr) [[unlikely]]
						ca::impl::assigned_over_an_uncleaned_object();
				}
				this->_release();
				   _object = ca::exchange(other._object, nullptr);
				_allocator = other._allocator;
				return *this;
			}


			// -- accessors --------------------------------------------------

			/* @DOC Whether it owns an object. */
			[[nodiscard]] constexpr auto holds(void) const noexcept -> bool {
				return _object != nullptr;
			}

			[[nodiscard]] constexpr explicit operator bool(void) const noexcept {
				return _object != nullptr;
			}

			/* @DOC The object, or nullptr. */
			[[nodiscard]] constexpr auto get(void) noexcept -> T* {
				return _object;
			}

			[[nodiscard]] constexpr auto get(void) const noexcept -> const T* {
				return _object;
			}

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}


			// -- element access ---------------------------------------------

			/* @DOC The object. Traps when there is none, in every build. */
			[[nodiscard]] constexpr auto operator*(void) noexcept -> T& {

				this->_expect_object();
				return *_object;
			}

			[[nodiscard]] constexpr auto operator*(void) const noexcept -> const T& {

				this->_expect_object();
				return *_object;
			}

			[[nodiscard]] constexpr auto operator->(void) noexcept -> T* {

				this->_expect_object();
				return _object;
			}

			[[nodiscard]] constexpr auto operator->(void) const noexcept -> const T* {

				this->_expect_object();
				return _object;
			}


			// -- methods ----------------------------------------------------

			/* @DOC A new owner of a copy, in memory from the same allocator: T's clone() when it is
			   clonable, else its copy constructor. Empty gives empty. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<T> || ca::is_copy_constructible<T> {

				if (_object == nullptr)
					return self{_allocator};
				auto block = _allocator.allocate(sizeof(T), alignof(T));
				if (!block) [[unlikely]]
					return block.propagate();
				T* const copy = static_cast<T*>(block->data());
				if constexpr (ca::is_clonable<T>) {
					auto cloned = _object->clone();
					if (!cloned) [[unlikely]] {
						_allocator.deallocate(block->with_size(sizeof(T)), alignof(T));
						return cloned.propagate();
					}
					ca::construct_at(copy, ca::move(*cloned));
				}
				else
					ca::construct_at(copy, *_object);
				return self{copy, _allocator};
			}

			/* @DOC Cleans up the object (cleanup() when T has one), destroys it and frees its memory:
			   empty after, even when cleanup fails; returns that failure. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				if (_object == nullptr)
					return ca::result<void>{};
				ca::result<void> cleaned{};
				if constexpr (ca::is_cleanable<T>)
					cleaned = _object->cleanup();
				this->_release();
				_object = nullptr;
				return cleaned;
			}


		private:

			// -- private methods --------------------------------------------

			constexpr auto _expect_object(void) const noexcept -> void {
				if (_object == nullptr) [[unlikely]]
					ca::impl::object_accessed_through_an_empty_owner();
			}

			auto _release(void) noexcept -> void {

				if (_object == nullptr)
					return;
				ca::destroy_at(_object);
				_allocator.deallocate(ca::allocation{_object, sizeof(T)}, alignof(T));
			}

	}; // class unique

} // namespace ca

#endif // ca_lifecycle_unique_hpp
