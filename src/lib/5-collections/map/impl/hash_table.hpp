#ifndef ca_collections_map_impl_hash_table_hpp
#define ca_collections_map_impl_hash_table_hpp

#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/memory/fill_bytes.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/memory/relocate_n.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/align_up.hpp"
#include "1-primitives/numeric/max.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_trivially_copyable.hpp"
#include "1-primitives/traits/is_trivially_destructible.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/result.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/map/hash.hpp"
#include "5-collections/map/impl/control_group.hpp"
#include "5-collections/map/impl/key.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IMPL ---------------------------------------------------------------

	namespace impl {


		// -- HASH TABLE -----------------------------------------------------

		/* @DOC Open-addressing table of Slot, keyed by K (the slot itself, or its key member): a
		   Swiss table. One block from A holds the control bytes, then the slots. Probing walks
		   aligned groups of 8 control bytes, triangularly, matching the hash's low 7 bits 8 slots
		   at a time; the rest of the hash picks the first group. At most 7/8 of the slots are
		   used or deleted. Erasing marks a slot EMPTY when its group already has one (no probe
		   ever went past it), DELETED otherwise; growth drops the DELETED. Insertion runs in two
		   steps so the caller builds the slot before it is counted: prepare finds the key or the
		   slot it goes in (growing first), commit claims it. */
		template <typename K, typename Slot, ca::is_allocator A>
		class hash_table final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::hash_table<K, Slot, A>;

				using group = ca::impl::control_group;

				using lookup = ca::impl::lookup_of<K>;

				struct layout final {

						using self = typename ca::impl::hash_table<K, Slot, A>::layout;

						ca::u8* _control;
						Slot*   _slots;

				}; // struct layout


				// -- private constants --------------------------------------

				static constexpr ca::usz _alignment = ca::max(alignof(Slot), alignof(ca::u64));

				static constexpr ca::usz _none = ca::limits<ca::usz>::max();


			public:

				// -- public types -------------------------------------------

				/* @DOC Where prepare found key (_found) or would put it: the slot index and the tag
				   its control byte takes. */
				struct place final {

						using self = typename ca::impl::hash_table<K, Slot, A>::place;

						ca::usz _index;
						ca::u8  _tag;
						bool    _found;

				}; // struct place


				// -- public constants ---------------------------------------

				/* @DOC Pointers, counts and the allocator; nothing points back at the table. */
				static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


			private:

				// -- private members ----------------------------------------

				ca::u8*                 _control;
				Slot*                   _slots;
				ca::usz                 _mask;
				ca::usz                 _size;
				ca::usz                 _growth;
				[[no_unique_address]] A _allocator;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr hash_table(void) noexcept requires ca::is_stateless_allocator<A>
				: _control{ca::impl::empty_control}, _slots{nullptr}, _mask{group::WIDTH - 1U}, _size{0U}, _growth{0U}, _allocator{} {
				}

				constexpr explicit hash_table(const A& allocator) noexcept
				: _control{ca::impl::empty_control}, _slots{nullptr}, _mask{group::WIDTH - 1U}, _size{0U}, _growth{0U}, _allocator{allocator} {
				}

				hash_table(const self&) = delete;

				constexpr hash_table(self&& other) noexcept
				: _control{ca::exchange(other._control, ca::impl::empty_control)}, _slots{ca::exchange(other._slots, nullptr)},
				  _mask{ca::exchange(other._mask, group::WIDTH - 1U)}, _size{ca::exchange(other._size, 0U)},
				  _growth{ca::exchange(other._growth, 0U)}, _allocator{other._allocator} {
				}

				// Failsafe for a skipped cleanup(): destroys the slots and frees, silently.
				// ~hash_table(void) noexcept {
				// 	this->_release();
				// }


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					if constexpr (ca::is_cleanable<Slot>) {
						if (_size != 0U) [[unlikely]]
							ca::impl::assigned_over_uncleaned_elements();
					}
					this->_release();
					  _control = ca::exchange(other._control, ca::impl::empty_control);
					    _slots = ca::exchange(other._slots, nullptr);
					     _mask = ca::exchange(other._mask, group::WIDTH - 1U);
					     _size = ca::exchange(other._size, 0U);
					   _growth = ca::exchange(other._growth, 0U);
					_allocator = other._allocator;
					return *this;
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
					return _size;
				}

				[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
					return _slots == nullptr ? 0U : _mask + 1U;
				}

				[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
					return _allocator;
				}

				[[nodiscard]] constexpr auto slot(const ca::usz index) const noexcept -> Slot* {
					return _slots + index;
				}

				[[nodiscard]] constexpr auto index_of(const Slot* const slot) const noexcept -> ca::usz {
					return static_cast<ca::usz>(slot - _slots);
				}

				[[nodiscard, gnu::always_inline]] static constexpr auto key_of(const Slot& slot) noexcept -> const K& {

					if constexpr (ca::is_same<Slot, K>)
						return slot;
					else
						return slot.key;
				}


				// -- methods ------------------------------------------------

				/* @DOC The slot holding key, or null. */
				[[nodiscard]] auto find(const lookup key) const noexcept -> Slot* {

					const ca::u64 hash = ca::hash(key);
					const ca::u8 tag = static_cast<ca::u8>(hash & 0x7FU);
					const ca::usz groups = _mask >> 3U;
					ca::usz at = static_cast<ca::usz>(hash >> 7U) & groups;
					for (ca::usz step = 1U;; ++step) {
						const group control{_control + (at << 3U)};
						for (ca::u64 match = control.match(tag); match != 0U; match &= match - 1U) {
							Slot* const slot = _slots + (at << 3U) + group::first(match);
							if (ca::impl::key_view(self::key_of(*slot)) == key) [[likely]]
								return slot;
						}
						if (control.match_empty() != 0U) [[likely]]
							return nullptr;
						at = (at + step) & groups;
					}
				}

				/* @DOC Where key is, or the free slot it goes in, growing first when that slot is
				   EMPTY and no growth is left. On failure nothing changes. */
				[[nodiscard]] auto prepare(const lookup key) noexcept -> ca::result<place> {

					const ca::u64 hash = ca::hash(key);
					const ca::u8 tag = static_cast<ca::u8>(hash & 0x7FU);
					const ca::usz groups = _mask >> 3U;
					ca::usz at = static_cast<ca::usz>(hash >> 7U) & groups;
					ca::usz free = self::_none;
					for (ca::usz step = 1U;; ++step) {
						const group control{_control + (at << 3U)};
						for (ca::u64 match = control.match(tag); match != 0U; match &= match - 1U) {
							const ca::usz index = (at << 3U) + group::first(match);
							if (ca::impl::key_view(self::key_of(_slots[index])) == key) [[likely]]
								return place{index, tag, true};
						}
						if (free == self::_none) {
							const ca::u64 open = control.match_free();
							if (open != 0U)
								free = (at << 3U) + group::first(open);
						}
						if (control.match_empty() != 0U) [[likely]]
							break;
						at = (at + step) & groups;
					}
					if (_growth == 0U && _control[free] == group::EMPTY) [[unlikely]] {
						auto grown = this->_grow();
						if (!grown)
							return grown.propagate();
						free = self::_free_slot(_control, _mask, hash);
					}
					return place{free, tag, false};
				}

				/* @DOC Counts the slot at spot, built by the caller since prepare, and returns it. */
				[[gnu::always_inline]] auto commit(const place spot) noexcept -> Slot* {

					if (_control[spot._index] == group::EMPTY)
						--_growth;
					_control[spot._index] = spot._tag;
					++_size;
					return _slots + spot._index;
				}

				/* @DOC Uncounts the slot at index, already destroyed by the caller. */
				[[gnu::always_inline]] auto vacate(const ca::usz index) noexcept -> void {

					if (group{_control + (index & ~(group::WIDTH - 1U))}.match_empty() != 0U) {
						_control[index] = group::EMPTY;
						++_growth;
					}
					else
						_control[index] = group::DELETED;
					--_size;
				}

				/* @DOC Index of the first used slot at or after from, or capacity(). */
				[[nodiscard]] auto next(ca::usz from) const noexcept -> ca::usz {

					const ca::usz capacity = this->capacity();
					while (from < capacity) {
						const ca::usz base = from & ~(group::WIDTH - 1U);
						const ca::u64 full = group{_control + base}.match_full() & (~ca::u64{0U} << ((from - base) << 3U));
						if (full != 0U)
							return base + group::first(full);
						from = base + group::WIDTH;
					}
					return capacity;
				}

				/* @DOC Room for count slots without growing. On failure nothing changes. */
				[[nodiscard]] auto reserve(const ca::usz count) noexcept -> ca::result<void> {

					if (count <= _size + _growth)
						return ca::result<void>{};
					const ca::usz capacity = self::_capacity_for(ca::max(count, _size));
					if (capacity == 0U) [[unlikely]]
						return ca::failure{ca::errc::out_of_memory};
					return this->_resize(capacity);
				}

				/* @DOC The smallest capacity holding size(), DELETED dropped; frees the storage when
				   empty. On failure nothing changes. */
				[[nodiscard]] auto shrink(void) noexcept -> ca::result<void> {

					if (_size == 0U) {
						this->_release();
						return ca::result<void>{};
					}
					const ca::usz capacity = self::_capacity_for(_size);
					if (capacity == this->capacity() && _size + _growth == self::_max_load(capacity))
						return ca::result<void>{};
					return this->_resize(capacity);
				}

				/* @DOC Drops the slots for which keep(slot) is false: cleanup, destructor, vacate.
				   Returns the first cleanup error. */
				template <typename F>
				[[nodiscard]] auto retain(F&& keep) noexcept -> ca::result<void> {

					ca::error first{};
					const ca::usz capacity = this->capacity();
					for (ca::usz index = this->next(0U); index < capacity; index = this->next(index + 1U)) {
						if (!keep(_slots[index])) {
							ca::impl::clean(_slots[index], first);
							ca::destroy_at(_slots + index);
							this->vacate(index);
						}
					}
					if (!first.ok())
						return ca::failure{first};
					return ca::result<void>{};
				}

				/* @DOC Cleans up and destroys every slot; keeps the storage. Returns the first
				   cleanup error; every slot is gone regardless. */
				[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {

					if (_slots == nullptr)
						return ca::result<void>{};
					ca::error first{};
					if constexpr (ca::is_cleanable<Slot> || !ca::is_trivially_destructible<Slot>) {
						const ca::usz capacity = this->capacity();
						for (ca::usz index = this->next(0U); index < capacity; index = this->next(index + 1U)) {
							ca::impl::clean(_slots[index], first);
							ca::destroy_at(_slots + index);
						}
					}
					const ca::usz capacity = this->capacity();
					ca::fill_bytes(_control, group::EMPTY, capacity);
					  _size = 0U;
					_growth = self::_max_load(capacity);
					if (!first.ok())
						return ca::failure{first};
					return ca::result<void>{};
				}

				/* @DOC clear(), then frees the storage. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

					auto cleaned = this->clear();
					this->_release();
					return cleaned;
				}

				/* @DOC A copy with the same capacity and placement, in memory from allocator: no
				   rehash. On a failed slot clone, the partial copy is destroyed and that error
				   returned. */
				[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self>
				requires ca::is_clonable<Slot> || ca::is_copy_constructible<Slot> {

					self copy{allocator};
					if (_size == 0U)
						return copy;
					const ca::usz capacity = this->capacity();
					auto block = self::_allocate(allocator, capacity);
					if (!block)
						return block.propagate();
					ca::copy_bytes(block->_control, _control, capacity);
					copy._control = block->_control;
					  copy._slots = block->_slots;
					   copy._mask = _mask;
					 copy._growth = _growth;
					if constexpr (ca::is_trivially_copyable<Slot> && !ca::is_clonable<Slot>) {
						ca::copy_bytes(copy._slots, _slots, capacity * sizeof(Slot));
						copy._size = _size;
					}
					else {
						for (ca::usz index = this->next(0U); index < capacity; index = this->next(index + 1U)) {
							auto cloned = ca::impl::make_clone(_slots[index]);
							if (!cloned) {
								ca::fill_bytes(copy._control + index, group::EMPTY, capacity - index);
								static_cast<void>(copy.cleanup());
								return cloned.propagate();
							}
							ca::construct_at(copy._slots + index, ca::move(*cloned));
							++copy._size;
						}
					}
					return copy;
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] static constexpr auto _max_load(const ca::usz capacity) noexcept -> ca::usz {
					return capacity - capacity / 8U;
				}

				[[nodiscard]] static constexpr auto _capacity_for(const ca::usz count) noexcept -> ca::usz {

					ca::usz capacity = group::WIDTH;
					while (self::_max_load(capacity) < count) {
						if (ca::mul_overflow(capacity, capacity, 2U)) [[unlikely]]
							return 0U;
					}
					return capacity;
				}

				[[nodiscard]] static auto _bytes(const ca::usz capacity, ca::usz& bytes) noexcept -> bool {

					ca::usz offset = capacity;
					ca::usz slots{0U};
					return ca::align_up(offset, alignof(Slot)) || ca::mul_overflow(slots, capacity, sizeof(Slot)) || ca::add_overflow(bytes, offset, slots);
				}

				[[nodiscard]] static auto _allocate(const A allocator, const ca::usz capacity) noexcept -> ca::result<layout> {

					ca::usz bytes{0U};
					if (self::_bytes(capacity, bytes)) [[unlikely]]
						return ca::failure{ca::errc::out_of_memory};
					auto block = allocator.allocate(bytes, self::_alignment);
					if (!block)
						return block.propagate();
					ca::u8* const control = static_cast<ca::u8*>(block->data());
					ca::fill_bytes(control, group::EMPTY, capacity);
					ca::usz offset = capacity;
					static_cast<void>(ca::align_up(offset, alignof(Slot)));
					return layout{control, static_cast<Slot*>(static_cast<void*>(control + offset))};
				}

				static auto _deallocate(const A allocator, ca::u8* const control, const ca::usz capacity) noexcept -> void {

					ca::usz bytes{0U};
					static_cast<void>(self::_bytes(capacity, bytes));
					allocator.deallocate(ca::allocation{control, bytes}, self::_alignment);
				}

				[[nodiscard]] static auto _free_slot(const ca::u8* const control, const ca::usz mask, const ca::u64 hash) noexcept -> ca::usz {

					const ca::usz groups = mask >> 3U;
					ca::usz at = static_cast<ca::usz>(hash >> 7U) & groups;
					for (ca::usz step = 1U;; ++step) {
						const ca::u64 open = group{control + (at << 3U)}.match_free();
						if (open != 0U)
							return (at << 3U) + group::first(open);
						at = (at + step) & groups;
					}
				}

				[[nodiscard, gnu::noinline]] static auto _moved(const A allocator, ca::u8* const control, Slot* const slots, const ca::usz capacity, const ca::usz into) noexcept
				-> ca::result<layout> {

					auto block = self::_allocate(allocator, into);
					if (!block)
						return block.propagate();
					for (ca::usz base = 0U; base < capacity; base += group::WIDTH) {
						for (ca::u64 full = group{control + base}.match_full(); full != 0U; full &= full - 1U) {
							Slot* const from = slots + base + group::first(full);
							const ca::u64 hash = ca::hash(ca::impl::key_view(self::key_of(*from)));
							const ca::usz index = self::_free_slot(block->_control, into - 1U, hash);
							block->_control[index] = static_cast<ca::u8>(hash & 0x7FU);
							ca::relocate_n(from, 1U, block->_slots + index);
						}
					}
					if (slots != nullptr)
						self::_deallocate(allocator, control, capacity);
					return block;
				}

				[[nodiscard]] auto _resize(const ca::usz capacity) noexcept -> ca::result<void> {

					auto moved = self::_moved(_allocator, _control, _slots, this->capacity(), capacity);
					if (!moved)
						return moved.propagate();
					_control = moved->_control;
					  _slots = moved->_slots;
					   _mask = capacity - 1U;
					 _growth = self::_max_load(capacity) - _size;
					return ca::result<void>{};
				}

				[[nodiscard]] auto _grow(void) noexcept -> ca::result<void> {

					const ca::usz capacity = this->capacity();
					if (capacity == 0U)
						return this->_resize(group::WIDTH);
					if (_size + 1U <= self::_max_load(capacity) / 2U)
						return this->_resize(capacity);
					ca::usz doubled{0U};
					if (ca::mul_overflow(doubled, capacity, 2U)) [[unlikely]]
						return ca::failure{ca::errc::out_of_memory};
					return this->_resize(doubled);
				}

				auto _release(void) noexcept -> void {

					if (_slots == nullptr)
						return;
					const ca::usz capacity = this->capacity();
					if constexpr (!ca::is_trivially_destructible<Slot>) {
						for (ca::usz index = this->next(0U); index < capacity; index = this->next(index + 1U))
							ca::destroy_at(_slots + index);
					}
					self::_deallocate(_allocator, _control, capacity);
					_control = ca::impl::empty_control;
					  _slots = nullptr;
					   _mask = group::WIDTH - 1U;
					   _size = 0U;
					 _growth = 0U;
				}

		}; // class hash_table


		// -- TABLE ITERATOR -------------------------------------------------

		/* @DOC Walks the used slots of a hash table in slot order, showing each through
		   Project::get(Slot&). Invalidated by any insertion that grows the table and by moving
		   the table; erasing other slots keeps it valid. */
		template <typename Table, typename Slot, typename Project>
		class table_iterator final {


			private:

				// -- private types ------------------------------------------

				using self = ca::impl::table_iterator<Table, Slot, Project>;


				// -- private members ----------------------------------------

				const Table* _table;
				ca::usz      _index;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr table_iterator(const Table* const table, const ca::usz index) noexcept
				: _table{table}, _index{index} {
				}


				// -- element access -----------------------------------------

				/* @DOC The slot, as Project shows it. Traps at end(), in every build. */
				[[nodiscard]] constexpr auto operator*(void) const noexcept -> decltype(auto) {

					if (_index >= _table->capacity()) [[unlikely]]
						ca::impl::index_out_of_bounds();
					return Project::get(static_cast<Slot&>(*_table->slot(_index)));
				}


				// -- methods ------------------------------------------------

				constexpr auto operator++(void) noexcept -> self& {

					_index = _table->next(_index + 1U);
					return *this;
				}

				[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
					return _index == other._index;
				}

		}; // class table_iterator

	} // namespace impl

} // namespace ca

#endif // ca_collections_map_impl_hash_table_hpp
