/* @DOC Performance of ca::set<K, A>.

   Layout: the hashmap's table with the keys as slots: control pointer, slot pointer, mask, size
   and growth left, 40 bytes on 64-bit, plus the allocator (nothing for ca::heap). One block
   holds a control byte per slot, then the keys. An empty set owns no memory.

   | Operation | Time | Notes |
   |---|---|---|
   | contains | O(1) expected | hash, one W-byte control load per group probed (W = 16 on x86-64, else 8), one key compare per tag hit |
   | add | amortized O(1) expected | the lookup, then an insert into the first free slot met |
   | discard, remove | O(1) expected | |
   | pop | O(capacity / W) worst | scans control groups from the start |
   | union_with, update | O(size + other.size) | |
   | intersection, difference, is_subset, is_disjoint | O(size of the side walked) | the smaller side when the result allows |
   | symmetric_difference | O(size + other.size) | |
   | iteration | O(capacity / W + size) | W control bytes per step |
   | clone | O(capacity) | same placement, no rehash; one memcpy for trivially copyable keys |

   Space: 1 + sizeof(K) bytes per slot; capacity a power of two from W, at most 7/8 used.

   Locality, hashing, relocation, copy and move, and measurements: as ca::hashmap, the same table
   with smaller slots. */

#ifndef ca_collections_map_set_hpp
#define ca_collections_map_set_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "1-primitives/memory/destroy_at.hpp"
#include "1-primitives/traits/is_array.hpp"
#include "1-primitives/traits/is_const.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_move_constructible.hpp"
#include "1-primitives/traits/is_object.hpp"
#include "1-primitives/utility/forward.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/is_cleanable.hpp"
#include "4-lifecycle/is_clonable.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "5-collections/map/is_hashable.hpp"
#include "5-collections/map/impl/hash_table.hpp"
#include "5-collections/map/impl/key.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SET ----------------------------------------------------------------

	namespace impl {

		/* @DOC Shows a set slot as its key, read-only. */
		struct project_self final {

				using self = ca::impl::project_self;

				template <typename K>
				[[nodiscard]] static constexpr auto get(const K& slot) noexcept -> const K& {
					return slot;
				}

		}; // struct project_self

	} // namespace impl


	/* @DOC Unordered set of K in memory from A, Python's set API over a Swiss table: add,
	   discard, remove, pop, contains (in), size (len), update, union_with (set.union, a keyword
	   here), intersection, difference, symmetric_difference, their _update forms, is_subset,
	   is_superset, is_disjoint, ==; clone is set.copy. Keys are looked up by lookup_type (an sv
	   for str keys); add builds the key from what it is given only when absent. K is
	   ca::hash-able and compares with == against its lookup type. Construction allocates
	   nothing; add, update, the set algebra, reserve and clone acquire memory and can fail.
	   Insertions may move every key: positions do not survive them. Clean up with cleanup(); the
	   destructor is a failsafe that frees silently. */
	template <typename K, ca::is_allocator A = ca::heap>
	class set final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::set<K, A>;

			using table = ca::impl::hash_table<K, K, A>;


		public:

			// -- public types -----------------------------------------------

			using key_type        = K;
			using value_type      = K;
			using lookup_type     = ca::impl::lookup_of<K>;
			using allocator_type  = A;
			using size_type       = ca::usz;
			using iterator        = ca::impl::table_iterator<table, const K, ca::impl::project_self>;
			using const_iterator  = iterator;

			/* @DOC The same set type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::set<K, B>;


			// -- public constants -------------------------------------------

			/* @DOC No pointer into itself: containers move a set with memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = table::TRIVIALLY_RELOCATABLE;


		private:

			// -- private members --------------------------------------------

			table _table;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr set(void) noexcept requires ca::is_stateless_allocator<A>
			: _table{} {
			}

			/* @DOC Empty, drawing memory from allocator. */
			constexpr explicit set(const A& allocator) noexcept
			: _table{allocator} {
			}

			set(const self&) = delete;

			constexpr set(self&& other) noexcept
			: _table{ca::move(other._table)} {
			}

			// Failsafe for a skipped cleanup(): destroys the keys and frees, silently.
			// ~set(void) noexcept = default;

			/* @DOC The distinct keys of keys: set(iterable). On failure the partial set is
			   destroyed and that error returned. */
			[[nodiscard]] static auto make(const ca::borrowed_slice<const K> keys) noexcept -> ca::result<self>
			requires ca::is_stateless_allocator<A> && ca::impl::is_buildable_from<K, const K&> {

				self made{};
				auto added = made.update(keys);
				if (!added) {
					static_cast<void>(made.cleanup());
					return added.propagate();
				}
				return made;
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's keys and allocator; the previous keys are released as by the
			   destructor. Traps when K is cleanable and keys remain: clear() or cleanup() first. */
			auto operator=(self&& other) noexcept -> self& {

				_table = ca::move(other._table);
				return *this;
			}


			// -- iterators --------------------------------------------------

			/* @DOC Every key once, read-only, in table order. */
			[[nodiscard]] auto begin(void) const noexcept -> iterator {
				return iterator{&_table, _table.next(0U)};
			}

			[[nodiscard]] auto end(void) const noexcept -> iterator {
				return iterator{&_table, _table.capacity()};
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
				return _table.size();
			}

			[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
				return _table.size() == 0U;
			}

			/* @DOC Slots in the storage, 7/8 of them usable; 0 when none is held. */
			[[nodiscard]] constexpr auto capacity(void) const noexcept -> ca::usz {
				return _table.capacity();
			}

			[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
				return _table.allocator();
			}


			// -- element access ---------------------------------------------

			/* @DOC key in set. */
			[[nodiscard]] auto contains(const lookup_type key) const noexcept -> bool {
				return _table.find(key) != nullptr;
			}

			/* @DOC The stored key equal to key, or nothing. */
			[[nodiscard]] auto get(const lookup_type key) const noexcept -> ca::optional<const K&> {

				const K* const found = _table.find(key);
				if (found == nullptr)
					return ca::nullopt;
				return ca::optional<const K&>{*found};
			}


			// -- modifiers --------------------------------------------------

			/* @DOC set.add: adds a K built from key unless an equal one is present. On failure
			   nothing changes. */
			template <typename Q>
			requires ca::impl::is_buildable_from<K, Q>
			[[nodiscard]] auto add(Q&& key) noexcept -> ca::result<void> {

				auto spot = _table.prepare(key);
				if (!spot)
					return spot.propagate();
				if (spot->_found)
					return ca::result<void>{};
				auto made = ca::impl::make_from<K>(ca::forward<Q>(key));
				if (!made)
					return made.propagate();
				ca::construct_at(_table.slot(spot->_index), ca::move(*made));
				_table.commit(*spot);
				return ca::result<void>{};
			}

			/* @DOC set.discard: removes key, cleaned up and destroyed; whether it was present. A
			   cleanup error is dropped: remove() hands the key over instead. */
			auto discard(const lookup_type key) noexcept -> bool {

				K* const found = _table.find(key);
				if (found == nullptr)
					return false;
				ca::error ignored{};
				ca::impl::clean(*found, ignored);
				this->_erase(found);
				return true;
			}

			/* @DOC set.remove: removes key and returns it, or nothing when absent (KeyError). */
			[[nodiscard]] auto remove(const lookup_type key) noexcept -> ca::optional<K> requires ca::is_move_constructible<K> {

				K* const found = _table.find(key);
				if (found == nullptr)
					return ca::nullopt;
				ca::optional<K> taken{ca::in_place, ca::move(*found)};
				this->_erase(found);
				return taken;
			}

			/* @DOC set.pop: removes and returns a key, the first in table order, or nothing when
			   empty. */
			[[nodiscard]] auto pop(void) noexcept -> ca::optional<K> requires ca::is_move_constructible<K> {

				const ca::usz index = _table.next(0U);
				if (index == _table.capacity())
					return ca::nullopt;
				K* const found = _table.slot(index);
				ca::optional<K> taken{ca::in_place, ca::move(*found)};
				this->_erase(found);
				return taken;
			}

			/* @DOC set.update: adds a copy of every key of keys. Stops at the first failure, the keys
			   added so far kept. */
			[[nodiscard]] auto update(const ca::borrowed_slice<const K> keys) noexcept -> ca::result<void> requires ca::impl::is_buildable_from<K, const K&> {

				auto reserved = _table.reserve(this->size() + keys.size());
				if (!reserved)
					return reserved.propagate();
				for (const K& key : keys) {
					auto added = this->add(key);
					if (!added)
						return added;
				}
				return ca::result<void>{};
			}

			[[nodiscard]] auto update(const self& other) noexcept -> ca::result<void> requires ca::impl::is_buildable_from<K, const K&> {

				if (this == &other)
					return ca::result<void>{};
				auto reserved = _table.reserve(this->size() + other.size());
				if (!reserved)
					return reserved.propagate();
				for (const K& key : other) {
					auto added = this->add(key);
					if (!added)
						return added;
				}
				return ca::result<void>{};
			}

			/* @DOC update, moving other's keys in; other is left empty, keys already present
			   cleaned up and destroyed. Stops at the first failure, the keys not moved yet left in
			   other. */
			[[nodiscard]] auto update(self&& other) noexcept -> ca::result<void> requires ca::is_move_constructible<K> {

				if (this == &other)
					return ca::result<void>{};
				auto reserved = _table.reserve(this->size() + other.size());
				if (!reserved)
					return reserved.propagate();
				for (ca::usz index = other._table.next(0U); index < other._table.capacity(); index = other._table.next(index + 1U)) {
					K* const moved = other._table.slot(index);
					if (this->contains(*moved)) {
						ca::error ignored{};
						ca::impl::clean(*moved, ignored);
						other._erase(moved);
						continue;
					}
					auto added = this->add(ca::move(*moved));
					other._erase(moved);
					if (!added)
						return added;
				}
				return ca::result<void>{};
			}

			/* @DOC set.intersection_update: keeps the keys also in other. Returns the first cleanup
			   error of the keys dropped. */
			[[nodiscard]] auto intersection_update(const self& other) noexcept -> ca::result<void> {
				return this->retain([&other](const K& key) {
					return other.contains(key);
				});
			}

			/* @DOC set.difference_update: drops the keys in other. Returns the first cleanup error of
			   the keys dropped. */
			[[nodiscard]] auto difference_update(const self& other) noexcept -> ca::result<void> {

				if (this == &other)
					return this->clear();
				return this->retain([&other](const K& key) {
					return !other.contains(key);
				});
			}

			/* @DOC set.symmetric_difference_update: drops the keys in other, adds other's keys it
			   lacked. Stops at the first failure, the keys handled so far kept. */
			[[nodiscard]] auto symmetric_difference_update(const self& other) noexcept -> ca::result<void> requires ca::impl::is_buildable_from<K, const K&> {

				if (this == &other)
					return this->clear();
				for (const K& key : other) {
					K* const found = _table.find(ca::impl::key_view(key));
					if (found != nullptr) {
						ca::error ignored{};
						ca::impl::clean(*found, ignored);
						this->_erase(found);
						continue;
					}
					auto added = this->add(key);
					if (!added)
						return added;
				}
				return ca::result<void>{};
			}

			/* @DOC Keeps the keys for which test(key) holds; the others are cleaned up and
			   destroyed. Returns the first cleanup error; they are gone regardless. */
			template <typename F>
			[[nodiscard]] auto retain(F&& test) noexcept -> ca::result<void> {
				return _table.retain([&test](K& slot) {
					return static_cast<bool>(test(static_cast<const K&>(slot)));
				});
			}

			/* @DOC Room for count keys in total without growing. On failure nothing changes. */
			[[nodiscard]] auto reserve(const ca::usz count) noexcept -> ca::result<void> {
				return _table.reserve(count);
			}

			/* @DOC Brings capacity down to what size() needs; frees the storage when empty. On
			   failure nothing changes. */
			[[nodiscard]] auto shrink(void) noexcept -> ca::result<void> {
				return _table.shrink();
			}

			/* @DOC Cleans up and destroys every key; keeps the capacity. Returns the first cleanup
			   error; every key is gone regardless. */
			[[nodiscard]] auto clear(void) noexcept -> ca::result<void> {
				return _table.clear();
			}

			/* @DOC clear(), then frees the storage. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {
				return _table.cleanup();
			}


			// -- methods ----------------------------------------------------

			/* @DOC set.union: a new set of the keys in either, in memory from this set's
			   allocator. */
			[[nodiscard]] auto union_with(const self& other) const noexcept -> ca::result<self>
			requires (ca::is_clonable<K> || ca::is_copy_constructible<K>) && ca::impl::is_buildable_from<K, const K&> {

				const self& larger = this->size() >= other.size() ? *this : other;
				const self& smaller = this->size() >= other.size() ? other : *this;
				auto made = larger.clone_in(_table.allocator());
				if (!made)
					return made;
				auto added = made->update(smaller);
				if (!added) {
					static_cast<void>(made->cleanup());
					return added.propagate();
				}
				return made;
			}

			/* @DOC set.intersection: a new set of the keys in both, walking the smaller. */
			[[nodiscard]] auto intersection(const self& other) const noexcept -> ca::result<self> requires ca::impl::is_buildable_from<K, const K&> {

				const self& larger = this->size() >= other.size() ? *this : other;
				const self& smaller = this->size() >= other.size() ? other : *this;
				return this->_filtered(smaller, [&larger](const K& key) {
					return larger.contains(key);
				});
			}

			/* @DOC set.difference: a new set of the keys in this one, not in other. */
			[[nodiscard]] auto difference(const self& other) const noexcept -> ca::result<self> requires ca::impl::is_buildable_from<K, const K&> {
				return this->_filtered(*this, [&other](const K& key) {
					return !other.contains(key);
				});
			}

			/* @DOC set.symmetric_difference: a new set of the keys in exactly one of them. */
			[[nodiscard]] auto symmetric_difference(const self& other) const noexcept -> ca::result<self> requires ca::impl::is_buildable_from<K, const K&> {

				auto made = this->difference(other);
				if (!made)
					return made;
				for (const K& key : other) {
					if (this->contains(key))
						continue;
					auto added = made->add(key);
					if (!added) {
						static_cast<void>(made->cleanup());
						return added.propagate();
					}
				}
				return made;
			}

			/* @DOC set.issubset (<=): every key is in other. */
			[[nodiscard]] auto is_subset(const self& other) const noexcept -> bool {

				if (this->size() > other.size())
					return false;
				for (const K& key : *this) {
					if (!other.contains(key))
						return false;
				}
				return true;
			}

			/* @DOC set.issuperset (>=): every key of other is in this one. */
			[[nodiscard]] auto is_superset(const self& other) const noexcept -> bool {
				return other.is_subset(*this);
			}

			/* @DOC set.isdisjoint: no key in common, walking the smaller. */
			[[nodiscard]] auto is_disjoint(const self& other) const noexcept -> bool {

				const self& larger = this->size() >= other.size() ? *this : other;
				const self& smaller = this->size() >= other.size() ? other : *this;
				for (const K& key : smaller) {
					if (larger.contains(key))
						return false;
				}
				return true;
			}

			/* @DOC Same keys. */
			[[nodiscard]] auto operator==(const self& other) const noexcept -> bool {
				return this->size() == other.size() && this->is_subset(other);
			}

			/* @DOC A copy in memory from the same allocator, same capacity, keys in the same slots:
			   set.copy. Keys are copied with clone() when they have one, else with the copy
			   constructor. On a failure, the partial copy is destroyed and that error returned. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> requires ca::is_clonable<K> || ca::is_copy_constructible<K> {
				return this->clone_in(_table.allocator());
			}

			/* @DOC clone, in memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> requires ca::is_clonable<K> || ca::is_copy_constructible<K> {

				auto copied = _table.clone_in(allocator);
				if (!copied)
					return copied.propagate();
				self copy{allocator};
				copy._table = ca::move(*copied);
				return copy;
			}


		private:

			// -- private methods --------------------------------------------

			template <typename F>
			[[nodiscard]] auto _filtered(const self& from, F&& keep) const noexcept -> ca::result<self> {

				self made{_table.allocator()};
				for (const K& key : from) {
					if (!keep(key))
						continue;
					auto added = made.add(key);
					if (!added) {
						static_cast<void>(made.cleanup());
						return added.propagate();
					}
				}
				return made;
			}

			auto _erase(K* const found) noexcept -> void {

				ca::destroy_at(found);
				_table.vacate(_table.index_of(found));
			}


			// -- assertions -------------------------------------------------

			static_assert(ca::is_object<K> && !ca::is_const<K> && !ca::is_array<K>, "ca::set<K>: K must be a non-const, non-array object type");

			static_assert(ca::is_hashable<ca::impl::lookup_of<K>>, "ca::set<K>: K must be ca::hash-able");

	}; // class set

} // namespace ca

#endif // ca_collections_map_set_hpp
