#ifndef ca_collections_concurrent_impl_mpmc_core_hpp
#define ca_collections_concurrent_impl_mpmc_core_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/isz.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/launder.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "5-collections/concurrent/impl/queue_parts.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MPMC CORE ----------------------------------------------------------

	namespace impl {

		/* @DOC One slot of an mpmc ring: its sequence number, then room for a T. The sequence is the
		   position a producer may claim it at (empty), or that position + 1 once filled. */
		template <typename T>
		struct mpmc_cell final {
				using self = ca::impl::mpmc_cell<T>;
				ca::atomic<ca::usz> _sequence;
				alignas(T) ca::byte _storage[sizeof(T)];
		}; // struct mpmc_cell

		/* @DOC The claim counters of an mpmc ring, producers' and consumers' on lines of their own. */
		struct mpmc_indices final {
				using self = ca::impl::mpmc_indices;
				alignas(ca::impl::queue_line) ca::atomic<ca::usz> _enqueue;
				alignas(ca::impl::queue_line) ca::atomic<ca::usz> _dequeue;
		}; // struct mpmc_indices

		/* @DOC An mpmc ring borrowed from its owner: the counters, mask + 1 cells. */
		template <typename T>
		struct mpmc_ring final {
				using self = ca::impl::mpmc_ring<T>;
				ca::impl::mpmc_indices* _indices;
				ca::impl::mpmc_cell<T>* _cells;
				ca::usz _mask;
		}; // struct mpmc_ring

		/* @DOC Constructs the cells of a new ring, each empty at its own position. */
		template <typename T>
		auto mpmc_start(const ca::impl::mpmc_ring<T> ring) noexcept -> void {
			for (ca::usz at = 0U; at <= ring._mask; ++at)
				ca::construct_at(&ring._cells[at]._sequence, at);
		}

		[[nodiscard]] inline auto mpmc_lag(const ca::usz sequence, const ca::usz position) noexcept -> ca::isz {
			return static_cast<ca::isz>(sequence - position);
		}

		/* @DOC Claims the next position whose cell is empty and builds a T there from args; false
		   when the ring is full (the cell at the next position still holds an element). */
		template <typename T, typename... Args>
		[[nodiscard]] auto mpmc_try_emplace(const ca::impl::mpmc_ring<T> ring, Args&&... args) noexcept -> bool {

			ca::usz position = ring._indices->_enqueue.template load<ca::memory_order::relaxed>();
			for (;;) {
				ca::impl::mpmc_cell<T>& cell = ring._cells[position & ring._mask];
				const ca::isz lag = ca::impl::mpmc_lag(cell._sequence.template load<ca::memory_order::acquire>(), position);
				if (lag == 0) {
					if (ring._indices->_enqueue.template compare_exchange_weak<ca::memory_order::relaxed>(position, position + 1U)) {
						ca::construct_at(reinterpret_cast<T*>(cell._storage), ca::forward<Args>(args)...);
						cell._sequence.template store<ca::memory_order::release>(position + 1U);
						return true;
					}
				} else if (lag < 0) {
					return false;
				} else {
					position = ring._indices->_enqueue.template load<ca::memory_order::relaxed>();
				}
			}
		}

		/* @DOC Claims the oldest filled cell and moves its element out; nothing when the ring is
		   empty (or its oldest element is still being written). */
		template <typename T>
		[[nodiscard]] auto mpmc_try_pop(const ca::impl::mpmc_ring<T> ring) noexcept -> ca::optional<T> {

			ca::usz position = ring._indices->_dequeue.template load<ca::memory_order::relaxed>();
			for (;;) {
				ca::impl::mpmc_cell<T>& cell = ring._cells[position & ring._mask];
				const ca::isz lag = ca::impl::mpmc_lag(cell._sequence.template load<ca::memory_order::acquire>(), position + 1U);
				if (lag == 0) {
					if (ring._indices->_dequeue.template compare_exchange_weak<ca::memory_order::relaxed>(position, position + 1U)) {
						T* const element = ca::launder(reinterpret_cast<T*>(cell._storage));
						ca::optional<T> value{ca::in_place, ca::move(*element)};
						ca::destroy_at(element);
						cell._sequence.template store<ca::memory_order::release>(position + ring._mask + 1U);
						return value;
					}
				} else if (lag < 0) {
					return ca::nullopt;
				} else {
					position = ring._indices->_dequeue.template load<ca::memory_order::relaxed>();
				}
			}
		}

		/* @DOC Elements claimed for writing and not yet claimed for reading, at some moment. */
		template <typename T>
		[[nodiscard]] auto mpmc_size(const ca::impl::mpmc_ring<T> ring) noexcept -> ca::usz {

			const ca::usz dequeue = ring._indices->_dequeue.template load<ca::memory_order::acquire>();
			return ring._indices->_enqueue.template load<ca::memory_order::acquire>() - dequeue;
		}

		/* @DOC Cleans up and destroys the elements of a ring nobody uses any more, keeping the
		   first error. */
		template <typename T>
		auto mpmc_release(const ca::impl::mpmc_ring<T> ring, ca::error& first) noexcept -> void {

			const ca::usz enqueue = ring._indices->_enqueue.template load<ca::memory_order::acquire>();
			for (ca::usz at = ring._indices->_dequeue.template load<ca::memory_order::acquire>(); at != enqueue; ++at)
				ca::impl::queue_release(ca::launder(reinterpret_cast<T*>(ring._cells[at & ring._mask]._storage)), first);
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_concurrent_impl_mpmc_core_hpp
