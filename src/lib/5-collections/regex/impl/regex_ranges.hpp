#ifndef ca_collections_regex_impl_regex_ranges_hpp
#define ca_collections_regex_impl_regex_ranges_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/unicode/impl/class_ranges.hpp"
#include "2-vocabulary/unicode/impl/point_range.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/regex/impl/regex_look.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX RANGES -------------------------------------------------------

	/* @DOC Sets of code points as sorted, disjoint ranges: what a character class becomes. */
	namespace impl {

		using regex_range = ca::unicode::impl::point_range;

		inline constexpr ca::impl::regex_range regex_ascii_word[4] = {{U'0', U'9'}, {U'A', U'Z'}, {U'_', U'_'}, {U'a', U'z'}};

		inline constexpr ca::impl::regex_range regex_ascii_digit[1] = {{U'0', U'9'}};

		inline constexpr ca::impl::regex_range regex_ascii_space[2] = {{U'\t', U'\r'}, {U' ', U' '}};

		/* @DOC The ranges of a named class, Unicode (Python's str classes) or ASCII. */
		[[nodiscard]] constexpr auto regex_class_ranges(const ca::impl::regex_class which, const bool ascii) noexcept -> ca::borrowed_slice<const ca::impl::regex_range> {

			switch (which) {
				case ca::impl::regex_class::word:
					return ascii ? ca::borrowed_slice<const ca::impl::regex_range>{ca::impl::regex_ascii_word}
						: ca::borrowed_slice<const ca::impl::regex_range>{ca::unicode::impl::word_ranges};
				case ca::impl::regex_class::digit:
					return ascii ? ca::borrowed_slice<const ca::impl::regex_range>{ca::impl::regex_ascii_digit}
						: ca::borrowed_slice<const ca::impl::regex_range>{ca::unicode::impl::decimal_ranges};
				case ca::impl::regex_class::space:
					return ascii ? ca::borrowed_slice<const ca::impl::regex_range>{ca::impl::regex_ascii_space}
						: ca::borrowed_slice<const ca::impl::regex_range>{ca::unicode::impl::space_ranges};
			}
			return {};
		}

		[[nodiscard]] constexpr auto regex_range_before(const ca::impl::regex_range& left, const ca::impl::regex_range& right) noexcept -> bool {
			return left._first < right._first || (left._first == right._first && left._last < right._last);
		}

		/* @DOC Heapsort by first point: in place, O(n log n), no allocation. */
		constexpr auto regex_sort_ranges(ca::impl::regex_range* const data, const ca::usz count) noexcept -> void {

			const auto swap = [data](const ca::usz left, const ca::usz right) noexcept -> void {

				const ca::impl::regex_range held = data[left];
				 data[left] = data[right];
				data[right] = held;
			};
			const auto sift = [data, &swap](ca::usz root, const ca::usz end) noexcept -> void {
				while (2U * root + 1U < end) {
					ca::usz child = 2U * root + 1U;
					if (child + 1U < end && ca::impl::regex_range_before(data[child], data[child + 1U]))
						++child;
					if (!ca::impl::regex_range_before(data[root], data[child]))
						return;
					swap(root, child);
					root = child;
				}
			};
			for (ca::usz i = count / 2U; i != 0U; --i)
				sift(i - 1U, count);
			for (ca::usz end = count; end > 1U; --end) {
				swap(0U, end - 1U);
				sift(0U, end - 1U);
			}
		}

		/* @DOC Sorts, then merges overlapping and adjacent ranges. */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_normalize(ca::dynamic_array<ca::impl::regex_range, A>& ranges) noexcept -> ca::result<void> {

			ca::impl::regex_range* const data = ranges.data();
			const ca::usz count = ranges.size();
			if (count < 2U)
				return ca::result<void>{};
			ca::impl::regex_sort_ranges(data, count);
			ca::usz kept = 0U;
			for (ca::usz i = 1U; i < count; ++i) {
				if (static_cast<ca::u32>(data[i]._first) <= static_cast<ca::u32>(data[kept]._last) + 1U) {
					if (data[i]._last > data[kept]._last)
						data[kept]._last = data[i]._last;
				}
				else
					data[++kept] = data[i];
			}
			return ranges.resize(kept + 1U);
		}

		/* @DOC Appends the complement of sorted, disjoint ranges within the scalar values. */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_append_complement(const ca::borrowed_slice<const ca::impl::regex_range> ranges, ca::dynamic_array<ca::impl::regex_range, A>& out) noexcept -> ca::result<void> {

			ca::u32 next = 0U;
			const auto gap = [&out](const ca::u32 first, const ca::u32 last) noexcept -> ca::result<void> {
				if (first <= 0xD7FFU && last >= 0xE000U) {
					auto low = out.append(ca::impl::regex_range{static_cast<char32_t>(first), U'퟿'});
					if (!low)
						return low;
					return out.append(ca::impl::regex_range{U'', static_cast<char32_t>(last)});
				}
				return out.append(ca::impl::regex_range{static_cast<char32_t>(first), static_cast<char32_t>(last)});
			};
			for (const ca::impl::regex_range& range : ranges) {
				const ca::u32 first = static_cast<ca::u32>(range._first);
				if (first > next) {
					auto added = gap(next, first - 1U);
					if (!added)
						return added;
				}
				next = static_cast<ca::u32>(range._last) + 1U;
			}
			if (next <= 0x10FFFFU)
				return gap(next, 0x10FFFFU);
			return ca::result<void>{};
		}

		/* @DOC Appends a named class, or its complement. */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_append_class(const ca::impl::regex_class which, const bool negated, const bool ascii, ca::dynamic_array<ca::impl::regex_range, A>& out) noexcept
		-> ca::result<void> {

			const ca::borrowed_slice<const ca::impl::regex_range> ranges = ca::impl::regex_class_ranges(which, ascii);
			if (negated)
				return ca::impl::regex_append_complement(ranges, out);
			return out.extend(ranges);
		}

		[[nodiscard]] constexpr auto regex_contains(const ca::borrowed_slice<const ca::impl::regex_range> ranges, const char32_t point) noexcept -> bool {

			ca::usz low = 0U;
			ca::usz high = ranges.size();
			while (low < high) {
				const ca::usz middle = low + (high - low) / 2U;
				if (ranges.data()[middle]._last < point)
					low = middle + 1U;
				else
					high = middle;
			}
			return low < ranges.size() && ranges.data()[low]._first <= point;
		}

		/* @DOC The index of point's link in the caseless cycles, or CASELESS_LINK_COUNT. */
		[[nodiscard]] constexpr auto regex_caseless_link(const char32_t point) noexcept -> ca::u32 {

			ca::u32 low = 0U;
			ca::u32 high = ca::unicode::impl::CASELESS_LINK_COUNT;
			while (low < high) {
				const ca::u32 middle = low + (high - low) / 2U;
				if (ca::unicode::impl::caseless_links[middle]._point < point)
					low = middle + 1U;
				else
					high = middle;
			}
			if (low < ca::unicode::impl::CASELESS_LINK_COUNT && ca::unicode::impl::caseless_links[low]._point == point)
				return low;
			return ca::unicode::impl::CASELESS_LINK_COUNT;
		}

		/* @DOC The other case of an ASCII letter, or point. */
		[[nodiscard]] constexpr auto regex_ascii_swap_case(const char32_t point) noexcept -> char32_t {

			if (point >= U'a' && point <= U'z')
				return static_cast<char32_t>(point - 32U);
			if (point >= U'A' && point <= U'Z')
				return static_cast<char32_t>(point + 32U);
			return point;
		}

		/* @DOC Appends the points re.IGNORECASE takes as point, point included: its caseless
		   class, or (ascii) the other case of an ASCII letter. */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_append_caseless(const char32_t point, const bool ascii, ca::dynamic_array<ca::impl::regex_range, A>& out) noexcept -> ca::result<void> {

			if (ascii) {
				auto added = out.append(ca::impl::regex_range{point, point});
				const char32_t other = ca::impl::regex_ascii_swap_case(point);
				if (!added || other == point)
					return added;
				return out.append(ca::impl::regex_range{other, other});
			}
			const ca::u32 link = ca::impl::regex_caseless_link(point);
			if (link == ca::unicode::impl::CASELESS_LINK_COUNT)
				return out.append(ca::impl::regex_range{point, point});
			char32_t member = point;
			do {
				auto added = out.append(ca::impl::regex_range{member, member});
				if (!added)
					return added;
				member = ca::unicode::impl::caseless_links[ca::impl::regex_caseless_link(member)]._next;
			} while (member != point);
			return ca::result<void>{};
		}

		/* @DOC Closes normalized ranges under re.IGNORECASE, then normalizes them again. */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_fold(ca::dynamic_array<ca::impl::regex_range, A>& ranges, const bool ascii) noexcept -> ca::result<void> {

			const ca::usz count = ranges.size();
			if (ascii) {
				for (char32_t upper = U'A'; upper <= U'Z'; ++upper) {
					const char32_t lower = ca::impl::regex_ascii_swap_case(upper);
					const ca::borrowed_slice<const ca::impl::regex_range> view{ranges.data(), count};
					if (ca::impl::regex_contains(view, upper) != ca::impl::regex_contains(view, lower)) {
						auto added = ranges.append(ca::impl::regex_range{upper, upper});
						if (!added || !(added = ranges.append(ca::impl::regex_range{lower, lower})))
							return added;
					}
				}
				return ca::impl::regex_normalize(ranges);
			}
			for (ca::u32 i = 0U; i < ca::unicode::impl::CASELESS_LINK_COUNT; ++i) {
				const ca::unicode::impl::caseless_link& link = ca::unicode::impl::caseless_links[i];
				const ca::borrowed_slice<const ca::impl::regex_range> view{ranges.data(), count};
				if (ca::impl::regex_contains(view, link._point) && !ca::impl::regex_contains(view, link._next)) {
					auto added = ca::impl::regex_append_caseless(link._point, false, ranges);
					if (!added)
						return added;
				}
			}
			return ca::impl::regex_normalize(ranges);
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_ranges_hpp
