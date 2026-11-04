#ifndef ca_collections_regex_impl_pike_vm_hpp
#define ca_collections_regex_impl_pike_vm_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/limits.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "5-collections/regex/impl/regex_program.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- PIKE VM ------------------------------------------------------------

	namespace impl {

		inline constexpr ca::usz regex_no_position = ca::limits<ca::usz>::max();

		/* @DOC The threads alive at one text position, in priority order: a sparse set of thread
		   keys (see regex_program), and each thread's capture slots, by key. */
		struct regex_threads final {

				using self = ca::impl::regex_threads;

				ca::u32*  _dense;
				ca::u32*  _sparse;
				ca::usz*  _slots;
				ca::u32   _count;

		}; // struct regex_threads

		/* @DOC One step of the closure walk: follow instruction target, or (slot != none) put
		   value back into slot. */
		struct regex_frame final {

				using self = ca::impl::regex_frame;

				ca::usz _value;
				ca::u32 _target;
				ca::u32 _slot;

		}; // struct regex_frame

		/* @DOC The matcher's memory for one program, carved from one block (regex_scratch_size
		   bytes, aligned for a usz) by regex_scratch_at: two thread lists, the slots being built,
		   the best match's slots, the walk's stack. Nothing allocates while matching. */
		struct regex_scratch final {

				using self = ca::impl::regex_scratch;

				ca::impl::regex_threads _lists[2];
				ca::usz*                _current;
				ca::usz*                _best;
				ca::impl::regex_frame*  _stack;

		}; // struct regex_scratch

		/* @DOC One search: text, where it starts (from), an empty match at forbid ruled out
		   (Python's must-advance after an empty match; regex_no_position for none), matches
		   starting at from only (anchored), or ending at the end only (full). */
		struct regex_search final {

				using self = ca::impl::regex_search;

				const char* _text;
				ca::usz     _size;
				ca::usz     _from;
				ca::usz     _forbid;
				bool        _anchored;
				bool        _full;

		}; // struct regex_search

		/* @DOC Into bytes, the scratch for keys thread keys (instructions << shift) and slots
		   per thread; false past memory. */
		[[nodiscard]] constexpr auto regex_scratch_size(const ca::usz keys, const ca::u32 slots, ca::usz& bytes) noexcept -> bool {

			ca::usz words{0U};
			ca::usz thread_slots{0U};
			ca::usz frames{0U};
			ca::usz indices{0U};
			if (ca::mul_overflow(thread_slots, keys, static_cast<ca::usz>(slots), 2U, sizeof(ca::usz)))
				return false;
			if (ca::mul_overflow(words, static_cast<ca::usz>(slots), 2U, sizeof(ca::usz)))
				return false;
			if (ca::mul_overflow(frames, keys + 1U, sizeof(ca::impl::regex_frame)))
				return false;
			if (ca::mul_overflow(indices, keys, 4U, sizeof(ca::u32)))
				return false;
			return !ca::add_overflow(bytes, thread_slots, words, frames, indices);
		}

		/* @DOC Lays scratch out over block (regex_scratch_size bytes) and clears the sparse
		   sets' indices. */
		auto regex_scratch_at(void* block, ca::usz keys, ca::u32 slots) noexcept -> ca::impl::regex_scratch;

		/* @DOC Runs program over the search; true when it matches, the slots of the match
		   (leftmost-first, as a backtracker in priority order finds it) in scratch._best:
		   start and end of each group, regex_no_position when it took no part. O(text *
		   program), each text byte read once. */
		[[nodiscard]] auto regex_execute(const ca::impl::regex_program& program, const ca::impl::regex_scratch& scratch, const ca::impl::regex_search& search) noexcept -> bool;

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_pike_vm_hpp
