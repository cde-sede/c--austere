/* @DOC Performance of ca::basic_str<C, A>.

   Layout: 24 bytes on 64-bit plus the allocator (nothing for a stateless one). A union holds
   either the text itself (small string: up to INLINE_CAPACITY units, 23 chars, 11 char16_t, 5
   char32_t) or a pointer, a size and a capacity. The last unit of the object tells them apart:
   inline, it holds INLINE_CAPACITY - size, which is 0 (the NUL) when full; allocated, it is the
   top of the capacity word, whose highest bit is set. Little-endian targets only.

   | Operation | Time | Notes |
   |---|---|---|
   | size, capacity, data, view, c_str, operator[] | O(1) | one test of the tag unit |
   | push | amortized O(1) | inline until INLINE_CAPACITY, then capacity doubles |
   | append, assign | amortized O(n) | one memcpy; on growth one more of the old text |
   | insert(i, text), remove(i, n) | O(size - i) | one memmove |
   | reserve, shrink | O(n) | a new block, then one memcpy |
   | clone | O(n) | no allocation when the text fits inline |
   | find, count, strip, split, partition, is_* | O(n) | views, no allocation; split is lazy |
   | upper, lower, title, ... | O(n) | one block of size() units; ASCII stored directly, other code points through the tables |
   | replace | O(n) | a counting pass, then one block of the exact size |
   | center, justify_*, zero_fill, repeat | O(n) | one block of the exact size |

   Always NUL-terminated: c_str() is data(), the allocation holds capacity + 1 units.

   Growth allocates the new block, copies, then frees the old one: appending or inserting a
   view of the string itself is safe, and a failed growth changes nothing. Capacity is what the
   allocator returns (the heap rounds to its size class).

   Copy and move: the copy constructor is deleted; clone() is the fallible copy. Moving copies
   the 24 bytes and the allocator and leaves the source empty. Relocatable with memcpy (no
   pointer into itself), so dynamic_array<basic_str> grows with realloc.

   Inlining: push, the fitting append, size, data and operator[] are inline; growth runs in
   out-of-line static helpers given pointers and sizes.

   Baseline std::string (libstdc++, glibc malloc); ca over ca::heap:
   --BENCH bench/string.cpp str
   bench/string.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | make() of 10 chars, 1e6 | 1.00x | 1.01x | inline in both; built inside the result: returning it copies nothing |
   | make() of 22 chars, 1e6 | 0.049x | 0.081x | std::string allocates past 15 |
   | push 1e7 chars | 1.04x | 0.89x | |
   | append 1e6 6-char words | 1.20x | 0.92x | |
   | ca::format vs std::format, short result, 1e6 | 0.63x | 0.54x | |
   | dynamic_array<str> vs vector<string>, 1e6 pushes, half inline | 0.78x | 0.87x | |
   | view().find of a missing 16-char needle in 1 MiB, 20 times | 0.99x | 1.02x | |
   BENCH--
   Python methods, against the equivalent hand-written std loops:
   --BENCH bench/string.cpp python
   bench/string.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | split(",") of 1 MiB, 20 times | 1.08x | 1.02x | std: a string_view::find loop |
   | upper() of 1 MiB ASCII, 20 times | 0.69x | 1.03x | std: push_back(toupper(c)) |
   | is_digit() of 1e6 short fields | 0.96x | 1.05x | std: an isdigit loop; ca: one table read per ASCII code point |
   BENCH-- */

#ifndef ca_collections_string_basic_str_hpp
#define ca_collections_string_basic_str_hpp

#include "1-primitives/bit/endian.hpp"
#include "1-primitives/integers/uint_of.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/memory/move_bytes.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/max.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "1-primitives/text/is_character.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/impl/access.hpp"
#include "2-vocabulary/in_place.hpp"
#include "2-vocabulary/nullopt.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "2-vocabulary/sv/impl/classify.hpp"
#include "2-vocabulary/sv/impl/methods.hpp"
#include "2-vocabulary/unicode/decoded.hpp"
#include "2-vocabulary/unicode/encode.hpp"
#include "2-vocabulary/unicode/mapping.hpp"
#include "2-vocabulary/unicode/max_units.hpp"
#include "2-vocabulary/unicode/properties.hpp"
#include "2-vocabulary/unicode/properties_of.hpp"
#include "2-vocabulary/unicode/transcode.hpp"
#include "2-vocabulary/unicode/transcoded_size.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/allocate_n.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/deallocate_n.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- BASIC STR ----------------------------------------------------------

	/* @DOC Growable, NUL-terminated string of code units C, in memory from A, short ones kept
	   inline (see INLINE_CAPACITY). Sizes count code units. Construction allocates nothing;
	   growing can fail and then changes nothing. Clean up with cleanup(); the destructor is a
	   failsafe that frees silently. Converts to sv::basic<C>. Python's str API: the
	   read-only methods (find, count, strip, split, partition, is_alpha, ...) return views
	   (sv::impl::methods); upper, lower, title, replace, center, ... return a new string
	   from the same allocator. operator[] traps out of range; get allows the doubt. Aliases: str, u8str, u16str,
	   u32str, wstr. */
	template <ca::is_character C, ca::is_allocator A = ca::heap>
	class basic_str final : public ca::sv::impl::methods<ca::basic_str<C, A>, C> {


		private:

			// -- private types ----------------------------------------------

			using self = ca::basic_str<C, A>;

			using unit = ca::uint_of<sizeof(C) * 8U>;

			struct allocated final {

					using self = typename ca::basic_str<C, A>::allocated;

					C*      _data;
					ca::usz _size;
					ca::usz _capacity;

			}; // struct allocated


			// -- private constants ------------------------------------------

			static constexpr ca::usz _units = sizeof(allocated) / sizeof(C);

			static constexpr ca::usz _long_flag = ca::usz{1U} << (sizeof(ca::usz) * 8U - 1U);

			static constexpr unit _tag_flag = static_cast<unit>(unit{1U} << (sizeof(C) * 8U - 1U));


		public:

			// -- public types -----------------------------------------------

			using value_type      = C;
			using allocator_type  = A;
			using size_type       = ca::usz;
			using reference       = C&;
			using const_reference = const C&;
			using pointer         = C*;
			using const_pointer   = const C*;
			using iterator        = C*;
			using const_iterator  = const C*;
			using view_type       = ca::sv::basic<C>;

			/* @DOC The same string type over allocator B. */
			template <ca::is_allocator B>
			using rebind = ca::basic_str<C, B>;


			// -- public constants -------------------------------------------

			/* @DOC Longest text held without allocating: 23 chars, 11 char16_t, 5 char32_t. */
			static constexpr ca::usz INLINE_CAPACITY = _units - 1U;

			/* @DOC No pointer into itself: containers move it with memcpy when A allows it. */
			static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


		private:

			// -- private members --------------------------------------------

			union {
					allocated _long;
					C _short[_units];
			};
			[[no_unique_address]] A _allocator;


		public:

			// -- public lifecycle -------------------------------------------

			basic_str(void) noexcept requires ca::is_stateless_allocator<A>
			: _short{}, _allocator{} {
				_short[_units - 1U] = static_cast<C>(INLINE_CAPACITY);
			}

			/* @DOC Empty, drawing memory from allocator. */
			explicit basic_str(const A& allocator) noexcept
			: _short{}, _allocator{allocator} {
				_short[_units - 1U] = static_cast<C>(INLINE_CAPACITY);
			}

			basic_str(const self&) = delete;

			basic_str(self&& other) noexcept
			: _short{}, _allocator{other._allocator} {

				ca::copy_bytes(_short, other._short, sizeof(allocated));
				other._reset();
			}

			// Failsafe for a skipped cleanup(): frees silently.
			// ~basic_str(void) noexcept {
			// 	this->_release();
			// }

			/* @DOC A copy of text, in memory from allocator (none when it fits inline). */
			[[nodiscard]] static auto make_in(const A& allocator, const view_type text) noexcept -> ca::result<self> {

				const ca::usz count = text.size();
				if (count <= INLINE_CAPACITY) [[likely]]
					return self::_from_inline(allocator, text);
				self out{allocator};
				auto block = self::_copied_block(allocator, text.data(), count, count);
				if (!block) [[unlikely]]
					return block.propagate();
				out._adopt(*block, count);
				return out;
			}

			[[nodiscard]] static auto make(const view_type text) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
				return self::make_in(A{}, text);
			}

			/* @DOC text transcoded from another encoding (invalid sequences become U+FFFD):
			   Python's str(bytes, encoding). */
			template <ca::is_character D>
			requires (!ca::is_same<D, C>)
			[[nodiscard]] static auto make_in(const A& allocator, const ca::sv::basic<D> text) noexcept -> ca::result<self> {

				const ca::usz size = ca::unicode::transcoded_size<C>(text);
				self out{allocator};
				auto reserved = out.reserve(size);
				if (!reserved) [[unlikely]]
					return reserved.propagate();
				out.set_size(ca::unicode::transcode(text, out.data(), size).written());
				return out;
			}

			template <ca::is_character D>
			requires (!ca::is_same<D, C>) && ca::is_stateless_allocator<A>
			[[nodiscard]] static auto make(const ca::sv::basic<D> text) noexcept -> ca::result<self> {
				return self::make_in(A{}, text);
			}


			// -- assignment operators ---------------------------------------

			auto operator=(const self&) -> self& = delete;

			/* @DOC Takes other's text and allocator; the previous text is freed. */
			auto operator=(self&& other) noexcept -> self& {

				if (this == &other)
					return *this;
				this->_release();
				ca::copy_bytes(_short, other._short, sizeof(allocated));
				_allocator = other._allocator;
				other._reset();
				return *this;
			}


			// -- iterators --------------------------------------------------

			[[nodiscard]] auto begin(void) noexcept -> C* {
				return this->data();
			}

			[[nodiscard]] auto begin(void) const noexcept -> const C* {
				return this->data();
			}

			[[nodiscard]] auto end(void) noexcept -> C* {
				return this->data() + this->size();
			}

			[[nodiscard]] auto end(void) const noexcept -> const C* {
				return this->data() + this->size();
			}


			// -- accessors --------------------------------------------------

			/* @DOC Length in code units, NUL excluded. */
			[[nodiscard]] auto size(void) const noexcept -> ca::usz {
				return this->_is_long() ? _long._size : INLINE_CAPACITY - static_cast<unit>(_short[_units - 1U]);
			}

			/* @DOC Units held without growing, NUL excluded. */
			[[nodiscard]] auto capacity(void) const noexcept -> ca::usz {
				return this->_is_long() ? _long._capacity & ~_long_flag : INLINE_CAPACITY;
			}

			[[nodiscard]] auto empty(void) const noexcept -> bool {
				return this->size() == 0U;
			}

			/* @DOC The units, followed by a NUL. */
			[[nodiscard]] auto data(void) noexcept -> C* {
				return this->_is_long() ? _long._data : _short;
			}

			[[nodiscard]] auto data(void) const noexcept -> const C* {
				return this->_is_long() ? _long._data : _short;
			}

			/* @DOC data(), for functions taking a NUL-terminated string. */
			[[nodiscard]] auto c_str(void) const noexcept -> const C* {
				return this->data();
			}

			[[nodiscard]] auto view(void) const noexcept -> view_type {
				return this->_is_long() ? view_type{_long._data, _long._size} : view_type{_short, INLINE_CAPACITY - static_cast<unit>(_short[_units - 1U])};
			}

			/* @DOC view(). */
			operator view_type(void) const noexcept {
				return this->view();
			}

			[[nodiscard]] auto allocator(void) const noexcept -> const A& {
				return _allocator;
			}


			// -- element access ---------------------------------------------

			/* @DOC Unit at index. Traps when index >= size(), in every build. */
			[[nodiscard]] auto operator[](const ca::usz index) noexcept -> C& {

				if (index >= this->size()) [[unlikely]]
					ca::impl::index_out_of_bounds();
				return this->data()[index];
			}

			[[nodiscard]] auto operator[](const ca::usz index) const noexcept -> const C& {

				if (index >= this->size()) [[unlikely]]
					ca::impl::index_out_of_bounds();
				return this->data()[index];
			}

			/* @DOC Unit at index, or nothing when index >= size(). */
			[[nodiscard]] auto get(const ca::usz index) noexcept -> ca::optional<C&> {

				if (index >= this->size())
					return ca::nullopt;
				return ca::optional<C&>{this->data()[index]};
			}

			[[nodiscard]] auto get(const ca::usz index) const noexcept -> ca::optional<const C&> {

				if (index >= this->size())
					return ca::nullopt;
				return ca::optional<const C&>{this->data()[index]};
			}


			// -- modifiers --------------------------------------------------

			/* @DOC Makes room for count units in total; never shrinks. On failure nothing changes. */
			[[nodiscard]] auto reserve(const ca::usz count) noexcept -> ca::result<void> {

				if (count <= this->capacity())
					return ca::result<void>{};
				const ca::usz size = this->size();
				auto block = self::_copied_block(_allocator, this->data(), size, count);
				if (!block) [[unlikely]]
					return block.propagate();
				this->_adopt(*block, size);
				return ca::result<void>{};
			}

			/* @DOC Brings capacity down to size(), moving the text inline when it fits there. On
			   failure nothing changes. */
			[[nodiscard]] auto shrink(void) noexcept -> ca::result<void> {

				if (!this->_is_long())
					return ca::result<void>{};
				const ca::usz size = _long._size;
				if (size <= INLINE_CAPACITY) {
					const allocated old = _long;
					ca::copy_bytes(_short, old._data, size * sizeof(C));
					this->_set_short_size(size);
					self::_free(_allocator, old._data, old._capacity & ~_long_flag);
					return ca::result<void>{};
				}
				if (size == (_long._capacity & ~_long_flag))
					return ca::result<void>{};
				auto block = self::_copied_block(_allocator, _long._data, size, size);
				if (!block) [[unlikely]]
					return block.propagate();
				this->_adopt(*block, size);
				return ca::result<void>{};
			}

			/* @DOC Appends c, growing when full. On failure nothing changes. */
			[[nodiscard]] auto push(const C c) noexcept -> ca::result<void> {

				if (!this->_is_long()) {
					const ca::usz size = INLINE_CAPACITY - static_cast<unit>(_short[_units - 1U]);
					if (size < INLINE_CAPACITY) [[likely]] {
						       _short[size] = c;
						  _short[size + 1U] = C{};
						_short[_units - 1U] = static_cast<C>(INLINE_CAPACITY - size - 1U);
						return ca::result<void>{};
					}
				}
				else if (_long._size < (_long._capacity & ~_long_flag)) [[likely]] {
					C* const data = _long._data;
					const ca::usz size = _long._size;
					     data[size] = c;
					data[size + 1U] = C{};
					    _long._size = size + 1U;
					return ca::result<void>{};
				}
				return this->_append_grown(&c, 1U);
			}

			/* @DOC Appends text, which may be a view of this string, growing geometrically. On
			   failure nothing changes. */
			[[nodiscard]] auto append(const view_type text) noexcept -> ca::result<void> {

				const ca::usz count = text.size();
				if (!this->_is_long()) {
					const ca::usz size = INLINE_CAPACITY - static_cast<unit>(_short[_units - 1U]);
					if (count <= INLINE_CAPACITY - size) [[likely]] {
						ca::copy_bytes(_short + size, text.data(), count * sizeof(C));
						this->_set_short_size(size + count);
						return ca::result<void>{};
					}
				}
				else {
					C* const data = _long._data;
					const ca::usz size = _long._size;
					if (count <= (_long._capacity & ~_long_flag) - size) [[likely]] {
						ca::copy_bytes(data + size, text.data(), count * sizeof(C));
						data[size + count] = C{};
						       _long._size = size + count;
						return ca::result<void>{};
					}
				}
				return this->_append_grown(text.data(), count);
			}

			/* @DOC Appends point encoded in C's encoding (U+FFFD when not a scalar value). */
			[[nodiscard]] auto append_code_point(const char32_t point) noexcept -> ca::result<void> {

				C units[ca::unicode::max_units<C>];
				return this->append(view_type{units, ca::unicode::encode(point, units)});
			}

			/* @DOC Replaces the text with text, which may be a view of this string. On failure
			   nothing changes. */
			[[nodiscard]] auto assign(const view_type text) noexcept -> ca::result<void> {

				const ca::usz count = text.size();
				if (count > this->capacity()) {
					auto block = self::_copied_block(_allocator, text.data(), count, count);
					if (!block) [[unlikely]]
						return block.propagate();
					this->_adopt(*block, count);
					return ca::result<void>{};
				}
				ca::move_bytes(this->data(), text.data(), count * sizeof(C));
				this->_set_size(count);
				return ca::result<void>{};
			}

			/* @DOC Inserts text at index, which may be a view of this string, shifting the rest.
			   Traps when index > size(), in every build. On failure nothing changes. */
			[[nodiscard]] auto insert(const ca::usz index, const view_type text) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				if (index > size) [[unlikely]]
					ca::impl::index_out_of_bounds();
				const ca::usz count = text.size();
				C* const data = this->data();
				const bool inside = text.data() >= data && text.data() <= data + size;
				if (count > this->capacity() - size || (inside && count != 0U)) {
					ca::usz total{0U};
					if (ca::add_overflow(total, size, count)) [[unlikely]]
						return ca::failure{ca::errc::out_of_memory};
					auto block = self::_copied_block(_allocator, data, index, ca::max(total, this->_next_capacity()));
					if (!block) [[unlikely]]
						return block.propagate();
					C* const into = static_cast<C*>(block->data());
					ca::copy_bytes(into + index, text.data(), count * sizeof(C));
					ca::copy_bytes(into + index + count, data + index, (size - index) * sizeof(C));
					this->_adopt(*block, total);
					return ca::result<void>{};
				}
				ca::move_bytes(data + index + count, data + index, (size - index) * sizeof(C));
				ca::copy_bytes(data + index, text.data(), count * sizeof(C));
				this->_set_size(size + count);
				return ca::result<void>{};
			}

			/* @DOC Removes up to count units from index, both clamped, shifting the rest. */
			auto remove(const ca::usz index, const ca::usz count) noexcept -> void {

				const ca::usz size = this->size();
				if (index >= size)
					return;
				const ca::usz removed = count < size - index ? count : size - index;
				C* const data = this->data();
				ca::move_bytes(data + index, data + index + removed, (size - index - removed) * sizeof(C));
				this->_set_size(size - removed);
			}

			/* @DOC Sets size() to count, new units set to fill (NUL by default). On failure nothing
			   changes. */
			[[nodiscard]] auto resize(const ca::usz count) noexcept -> ca::result<void> {
				return this->resize(count, C{});
			}

			[[nodiscard]] auto resize(const ca::usz count, const C fill) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				if (count > size) {
					auto reserved = this->reserve(count);
					if (!reserved) [[unlikely]]
						return reserved;
					C* const data = this->data();
					for (ca::usz i = size; i < count; ++i)
						data[i] = fill;
				}
				this->_set_size(count);
				return ca::result<void>{};
			}

			/* @DOC Sets size() to count without touching the units, for code that wrote them into
			   data() up to capacity() (a writer formatting in place). Writes the NUL. Traps when
			   count > capacity(), in every build. */
			auto set_size(const ca::usz count) noexcept -> void {

				if (count > this->capacity()) [[unlikely]]
					ca::impl::index_out_of_bounds();
				this->_set_size(count);
			}

			/* @DOC Removes and returns the last unit, or nothing when empty. */
			[[nodiscard]] auto pop(void) noexcept -> ca::optional<C> {

				const ca::usz size = this->size();
				if (size == 0U)
					return ca::nullopt;
				const C last = this->data()[size - 1U];
				this->_set_size(size - 1U);
				return last;
			}

			/* @DOC Empties the text; keeps the capacity. */
			auto clear(void) noexcept -> void {
				this->_set_size(0U);
			}

			/* @DOC Frees the storage: empty, inline. Cannot fail. */
			[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

				this->_release();
				this->_reset();
				return ca::result<void>{};
			}


			// -- methods ----------------------------------------------------

			/* @DOC A copy in memory from the same allocator. */
			[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> {
				return self::make_in(_allocator, this->view());
			}

			/* @DOC clone, in memory from allocator. */
			[[nodiscard]] auto clone_in(const A& allocator) const noexcept -> ca::result<self> {
				return self::make_in(allocator, this->view());
			}

			[[nodiscard]] auto operator==(const self& other) const noexcept -> bool {
				return this->view() == other.view();
			}

			[[nodiscard]] auto operator==(const view_type other) const noexcept -> bool {
				return this->view() == other;
			}


			// -- python methods ---------------------------------------------

			/* @DOC Each code point by its full uppercase mapping: str.upper. Invalid sequences
			   are copied as they are. */
			[[nodiscard]] auto upper(void) const noexcept -> ca::result<self> {
				return this->_mapped([](const ca::unicode::properties p) noexcept { return p.upper(); });
			}

			/* @DOC str.lower (Greek final sigma comes with the Unicode tables). */
			[[nodiscard]] auto lower(void) const noexcept -> ca::result<self> {
				return this->_mapped([](const ca::unicode::properties p) noexcept { return p.lower(); });
			}

			/* @DOC Full case folding, for caseless comparison: str.casefold. */
			[[nodiscard]] auto case_fold(void) const noexcept -> ca::result<self> {
				return this->_mapped([](const ca::unicode::properties p) noexcept { return p.fold(); });
			}

			/* @DOC Upper to lower and lower to upper: str.swapcase. */
			[[nodiscard]] auto swap_case(void) const noexcept -> ca::result<self> {
				return this->_mapped([](const ca::unicode::properties p) noexcept {
					return p.is_upper() ? p.lower() : p.is_lower() ? p.upper() : ca::unicode::mapping{p.code_point()};
				});
			}

			/* @DOC Titlecase after an uncased code point, lowercase after a cased one: str.title. */
			[[nodiscard]] auto title(void) const noexcept -> ca::result<self> {

				bool previous_cased = false;
				return this->_mapped([&previous_cased](const ca::unicode::properties p) noexcept {
					const ca::unicode::mapping mapped = previous_cased ? p.lower() : p.title();
					previous_cased = p.is_cased();
					return mapped;
				});
			}

			/* @DOC The first code point titlecased, the rest lowercased: str.capitalize. */
			[[nodiscard]] auto capitalize(void) const noexcept -> ca::result<self> {

				bool first = true;
				return this->_mapped([&first](const ca::unicode::properties p) noexcept {
					const ca::unicode::mapping mapped = first ? p.title() : p.lower();
					first = false;
					return mapped;
				});
			}

			/* @DOC Every occurrence of old replaced by replacement, or the first max:
			   str.replace. An empty old inserts replacement before every code point and at the
			   end. */
			[[nodiscard]] auto replace(const view_type old, const view_type replacement) const noexcept -> ca::result<self> {
				return this->replace(old, replacement, ~ca::usz{0U});
			}

			[[nodiscard]] auto replace(const view_type old, const view_type replacement, const ca::usz max) const noexcept -> ca::result<self> {

				const view_type text = this->view();
				const ca::usz found = old.empty() ? this->code_point_count() + 1U : this->count(old);
				const ca::usz cuts = found < max ? found : max;
				ca::usz room{0U};
				if (ca::mul_overflow(room, cuts, replacement.size()) || ca::add_overflow(room, room, text.size())) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				self out{_allocator};
				auto reserved = out.reserve(room);
				if (!reserved) [[unlikely]]
					return reserved.propagate();
				const C* at = text.data();
				const C* const end = text.data() + text.size();
				for (ca::usz left = cuts; left != 0U; --left) {
					const C* next = end;
					const C* resume = end;
					if (old.empty()) {
						  next = at;
						resume = at == end ? end : at + ca::sv::impl::point_at(at, end).length();
					}
					else {
						const ca::optional<ca::usz> hit = view_type{at, static_cast<ca::usz>(end - at)}.find(old);
						if (!hit)
							break;
						  next = at + *hit;
						resume = next + old.size();
					}
					auto appended = out.append(view_type{at, static_cast<ca::usz>(next - at)});
					if (appended)
						appended = out.append(replacement);
					if (appended && old.empty())
						appended = out.append(view_type{next, static_cast<ca::usz>(resume - next)});
					if (!appended) [[unlikely]]
						return appended.propagate();
					at = resume;
				}
				auto rest = out.append(view_type{at, static_cast<ca::usz>(end - at)});
				if (!rest) [[unlikely]]
					return rest.propagate();
				return out;
			}

			/* @DOC The text count times: Python's s * count. */
			[[nodiscard]] auto repeat(const ca::usz count) const noexcept -> ca::result<self> {

				ca::usz room{0U};
				if (ca::mul_overflow(room, count, this->size())) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				self out{_allocator};
				auto reserved = out.reserve(room);
				if (!reserved) [[unlikely]]
					return reserved.propagate();
				for (ca::usz i = 0U; i < count; ++i)
					static_cast<void>(out.append(this->view()));
				return out;
			}

			/* @DOC Centered in width code points, padded with fill (space by default):
			   str.center. Never cut. */
			[[nodiscard]] auto center(const ca::usz width) const noexcept -> ca::result<self> {
				return this->center(width, U' ');
			}

			[[nodiscard]] auto center(const ca::usz width, const char32_t fill) const noexcept -> ca::result<self> {

				const ca::usz length = this->code_point_count();
				const ca::usz margin = width > length ? width - length : 0U;
				const ca::usz left = margin / 2U + (margin & width & 1U);
				return this->_padded(fill, left, margin - left);
			}

			/* @DOC Padded at the end to width code points: str.ljust. */
			[[nodiscard]] auto justify_left(const ca::usz width) const noexcept -> ca::result<self> {
				return this->justify_left(width, U' ');
			}

			[[nodiscard]] auto justify_left(const ca::usz width, const char32_t fill) const noexcept -> ca::result<self> {

				const ca::usz length = this->code_point_count();
				return this->_padded(fill, 0U, width > length ? width - length : 0U);
			}

			/* @DOC Padded at the start to width code points: str.rjust. */
			[[nodiscard]] auto justify_right(const ca::usz width) const noexcept -> ca::result<self> {
				return this->justify_right(width, U' ');
			}

			[[nodiscard]] auto justify_right(const ca::usz width, const char32_t fill) const noexcept -> ca::result<self> {

				const ca::usz length = this->code_point_count();
				return this->_padded(fill, width > length ? width - length : 0U, 0U);
			}

			/* @DOC Zeros in front up to width code points, after a leading sign: str.zfill. */
			[[nodiscard]] auto zero_fill(const ca::usz width) const noexcept -> ca::result<self> {

				const ca::usz length = this->code_point_count();
				auto out = this->_padded(U'0', width > length ? width - length : 0U, 0U);
				if (out && width > length && length != 0U) {
					C* const data = out->data();
					const C sign = data[width - length];
					if (sign == C{'+'} || sign == C{'-'}) {
						             data[0] = sign;
						data[width - length] = C{'0'};
					}
				}
				return out;
			}

			/* @DOC Tabs replaced by spaces up to the next multiple of size columns (8 by
			   default), columns restarting after \n and \r: str.expandtabs. */
			[[nodiscard]] auto expand_tabs(void) const noexcept -> ca::result<self> {
				return this->expand_tabs(8U);
			}

			[[nodiscard]] auto expand_tabs(const ca::usz size) const noexcept -> ca::result<self> {

				const view_type text = this->view();
				self out{_allocator};
				auto reserved = out.reserve(text.size());
				if (!reserved) [[unlikely]]
					return reserved.propagate();
				ca::usz column = 0U;
				for (const C* at = text.data(), * const end = at + text.size(); at != end;) {
					const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
					ca::result<void> appended{};
					if (point.code_point() == U'\t') {
						const ca::usz spaces = size == 0U ? 0U : size - column % size;
						appended = out.resize(out.size() + spaces, C{' '});
						  column += spaces;
					}
					else {
						appended = out.append(view_type{at, point.length()});
						  column = point.code_point() == U'\n' || point.code_point() == U'\r' ? 0U : column + 1U;
					}
					if (!appended) [[unlikely]]
						return appended.propagate();
					at += point.length();
				}
				return out;
			}


		private:

			// -- private methods --------------------------------------------

			[[nodiscard, gnu::always_inline]] auto _is_long(void) const noexcept -> bool {
				return (static_cast<unit>(_short[_units - 1U]) & _tag_flag) != 0U;
			}

			auto _set_short_size(const ca::usz size) noexcept -> void {

				       _short[size] = C{};
				_short[_units - 1U] = static_cast<C>(INLINE_CAPACITY - size);
			}

			auto _set_size(const ca::usz size) noexcept -> void {

				if (this->_is_long()) {
					_long._data[size] = C{};
					      _long._size = size;
				}
				else
					this->_set_short_size(size);
			}

			auto _reset(void) noexcept -> void {

				          _short[0] = C{};
				_short[_units - 1U] = static_cast<C>(INLINE_CAPACITY);
			}

			auto _release(void) noexcept -> void {
				if (this->_is_long())
					self::_free(_allocator, _long._data, _long._capacity & ~_long_flag);
			}

			auto _adopt(const ca::allocation block, const ca::usz size) noexcept -> void {

				this->_release();
				C* const data = static_cast<C*>(block.data());
				data[size] = C{};
				     _long = allocated{data, size, (block.size() / sizeof(C) - 1U) | _long_flag};
			}

			/* @DOC Each code point through convert (properties -> mapping), invalid sequences copied.
			   ASCII mapped to ASCII is stored straight into the reserved block. */
			template <typename F>
			[[nodiscard]] auto _mapped(F&& convert) const noexcept -> ca::result<self> {

				const view_type text = this->view();
				self out{_allocator};
				auto reserved = out.reserve(text.size());
				if (!reserved) [[unlikely]]
					return reserved.propagate();
				C* into = out.data();
				ca::usz size = 0U;
				ca::usz capacity = out.capacity();
				for (const C* at = text.data(), * const end = at + text.size(); at != end;) {
					const ca::unicode::decoded point = ca::sv::impl::point_at(at, end);
					if (point.valid() && point.code_point() < U'\x80' && size < capacity) [[likely]] {
						const ca::unicode::mapping mapped = convert(ca::unicode::properties_of(point.code_point()));
						if (mapped.size() == 1U && mapped.first() < U'\x80') [[likely]] {
							into[size++] = static_cast<C>(mapped.first());
							++at;
							continue;
						}
					}
					out.set_size(size);
					ca::result<void> appended{};
					if (!point.valid())
						appended = out.append(view_type{at, point.length()});
					else {
						for (const char32_t mapped : convert(ca::unicode::properties_of(point.code_point()))) {
							appended = out.append_code_point(mapped);
							if (!appended) [[unlikely]]
								break;
						}
					}
					if (!appended) [[unlikely]]
						return appended.propagate();
					    into = out.data();
					    size = out.size();
					capacity = out.capacity();
					at += point.length();
				}
				out.set_size(size);
				return out;
			}

			[[nodiscard]] auto _padded(const char32_t fill, const ca::usz before, const ca::usz after) const noexcept -> ca::result<self> {

				C units[ca::unicode::max_units<C>];
				const ca::usz fill_size = ca::unicode::encode(fill, units);
				ca::usz room{0U};
				if (ca::add_overflow(room, before, after) || ca::mul_overflow(room, room, fill_size) || ca::add_overflow(room, room, this->size())) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				self out{_allocator};
				auto reserved = out.reserve(room);
				if (!reserved) [[unlikely]]
					return reserved.propagate();
				for (ca::usz i = 0U; i < before; ++i)
					static_cast<void>(out.append(view_type{units, fill_size}));
				static_cast<void>(out.append(this->view()));
				for (ca::usz i = 0U; i < after; ++i)
					static_cast<void>(out.append(view_type{units, fill_size}));
				return out;
			}

			/* @DOC Built inside the result, a single return: no copy of the 24 bytes, whose narrow
			   stores a wide reload would wait on. */
			[[nodiscard]] static auto _from_inline(const A& allocator, const view_type text) noexcept -> ca::result<self> {

				ca::result<self> made{ca::in_place, allocator};
				if (made) [[likely]] {
					ca::copy_bytes(made->_short, text.data(), text.size() * sizeof(C));
					made->_set_short_size(text.size());
				}
				return made;
			}

			[[nodiscard]] auto _next_capacity(void) const noexcept -> ca::usz {

				ca::usz doubled{0U};
				if (ca::mul_overflow(doubled, this->capacity() + 1U, 2U))
					return ca::usz{_long_flag - 1U};
				return doubled - 1U;
			}

			[[nodiscard, gnu::noinline]] auto _append_grown(const C* const text, const ca::usz count) noexcept -> ca::result<void> {

				const ca::usz size = this->size();
				ca::usz total{0U};
				if (ca::add_overflow(total, size, count)) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				auto block = self::_copied_block(_allocator, this->data(), size, ca::max(total, this->_next_capacity()));
				if (!block) [[unlikely]]
					return block.propagate();
				ca::copy_bytes(static_cast<C*>(block->data()) + size, text, count * sizeof(C));
				this->_adopt(*block, total);
				return ca::result<void>{};
			}

			/* @DOC A block for capacity units and the NUL, the size units at data copied in. */
			[[nodiscard, gnu::noinline]] static auto _copied_block(const A allocator, const C* const data, const ca::usz size, const ca::usz capacity) noexcept
			-> ca::result<ca::allocation> {

				ca::usz units{0U};
				if (capacity >= _long_flag || ca::add_overflow(units, capacity, 1U)) [[unlikely]]
					return ca::failure{ca::errc::out_of_memory};
				auto block = ca::allocate_n<C>(allocator, units);
				if (!block) [[unlikely]]
					return block.propagate();
				ca::copy_bytes(block->data(), data, size * sizeof(C));
				return block;
			}

			static auto _free(const A allocator, C* const data, const ca::usz capacity) noexcept -> void {
				ca::deallocate_n<C>(allocator, ca::allocation{data, (capacity + 1U) * sizeof(C)});
			}

			// -- assertions -------------------------------------------------

			static_assert(ca::endian::native == ca::endian::little, "ca::basic_str: the inline tag layout assumes a little-endian target");
			static_assert(sizeof(allocated) % sizeof(C) == 0U && sizeof(C) <= sizeof(ca::usz));

	}; // class basic_str

} // namespace ca

#endif // ca_collections_string_basic_str_hpp
