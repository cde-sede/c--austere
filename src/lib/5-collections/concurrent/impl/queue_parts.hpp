#ifndef ca_collections_concurrent_impl_queue_parts_hpp
#define ca_collections_concurrent_impl_queue_parts_hpp

#include "1-primitives/bit/bit_ceil.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/align_up.hpp"
#include "1-primitives/numeric/max.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/is_cleanable.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- QUEUE PARTS --------------------------------------------------------

	namespace impl {

		/* @DOC What separates indices written by different threads: two cache lines, since x86
		   prefetches lines in pairs and would otherwise make neighbours share one. */
		inline constexpr ca::usz queue_line = 128U;

		/* @DOC Where a queue's slots start and how large its block is: a header of size header,
		   then count slots of slot bytes aligned to alignment. */
		struct queue_block final {
				using self = ca::impl::queue_block;
				ca::usz _slots_at;
				ca::usz _size;
				ca::usz _alignment;
		}; // struct queue_block

		/* @DOC The power of two at least requested and at least minimum: errc::invalid_argument for
		   0, errc::out_of_memory past the largest. */
		[[nodiscard]] inline auto queue_capacity(const ca::usz requested, const ca::usz minimum) noexcept -> ca::result<ca::usz> {

			if (requested == 0U) [[unlikely]]
				return ca::failure{ca::errc::invalid_argument};
			if (requested > (ca::limits<ca::usz>::max() >> 1U) + 1U) [[unlikely]]
				return ca::failure{ca::errc::out_of_memory};
			return ca::bit_ceil(ca::max(requested, minimum));
		}

		/* @DOC The layout of a block holding header bytes then count slots; errc::out_of_memory on
		   overflow. */
		[[nodiscard]] inline auto queue_layout(const ca::usz header, const ca::usz count, const ca::usz slot, const ca::usz slot_alignment) noexcept -> ca::result<ca::impl::queue_block> {

			ca::usz slots_at = header;
			ca::usz bytes{0U};
			ca::usz size{0U};
			if (ca::align_up(slots_at, slot_alignment) || ca::mul_overflow(bytes, count, slot) || ca::add_overflow(size, slots_at, bytes)) [[unlikely]]
				return ca::failure{ca::errc::out_of_memory};
			return ca::impl::queue_block{slots_at, size, ca::max(ca::impl::queue_line, slot_alignment)};
		}

		/* @DOC queue_layout of a block already made, which cannot overflow: what its release needs. */
		[[nodiscard]] inline auto queue_layout_of(const ca::usz header, const ca::usz count, const ca::usz slot, const ca::usz slot_alignment) noexcept -> ca::impl::queue_block {

			ca::usz slots_at = header;
			static_cast<void>(ca::align_up(slots_at, slot_alignment));
			return ca::impl::queue_block{slots_at, slots_at + count * slot, ca::max(ca::impl::queue_line, slot_alignment)};
		}

		/* @DOC Cleans up (when T has cleanup) and destroys element, keeping the first error. */
		template <typename T>
		auto queue_release(T* const element, ca::error& first) noexcept -> void {

			if constexpr (ca::is_cleanable<T>) {
				element->cleanup().match([] {
				}, [&first](const ca::error& error) {
					if (first.ok())
						first = error;
				});
			}
			ca::destroy_at(element);
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_concurrent_impl_queue_parts_hpp
