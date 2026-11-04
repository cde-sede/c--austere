#ifndef ca_collections_regex_impl_regex_program_hpp
#define ca_collections_regex_impl_regex_program_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX PROGRAM ------------------------------------------------------

	namespace impl {

		enum class regex_op : ca::u8 {
			bytes,
			split,
			save,
			look,
			progress,
			match,
		};

		/* @DOC One instruction of a Thompson NFA over UTF-8 bytes. bytes: transitions x to x + y,
		   sorted, disjoint (none: a dead end); split: x first, then y; save: position into slot
		   y, then x; look: the assertion look, then x; progress: x when the position is still
		   the one in slot y (an iteration that matched empty leaves its loop, as in Python),
		   else z; match. */
		struct regex_inst final {

				using self = ca::impl::regex_inst;

				ca::u32            _x;
				ca::u32            _y;
				ca::u32            _z;
				ca::impl::regex_op _op;
				ca::u8             _look;

		}; // struct regex_inst

		/* @DOC A byte in low..high goes to instruction next. */
		struct regex_transition final {

				using self = ca::impl::regex_transition;

				ca::u32 _next;
				ca::u8  _low;
				ca::u8  _high;

		}; // struct regex_transition

		/* @DOC A capture group's name: name_size units at name_at in the names text. */
		struct regex_name final {

				using self = ca::impl::regex_name;

				ca::u32 _name_at;
				ca::u32 _name_size;
				ca::u32 _group;

		}; // struct regex_name

		/* @DOC What the matcher reads: the instructions, transitions, the entry, slots per thread
		   (two per group, group 0 included, then one per loop that can iterate empty), the bytes
		   a match can start with (first_bytes, a 256-bit set; all when skips is false), a
		   literal every match starts with (prefix), and whether matches only start at the
		   search position (anchored). guards (with shift 1; none with shift 0) give each
		   instruction the slot of the innermost loop that can iterate empty around it: a thread
		   is told apart by its instruction and whether that loop's iteration is still empty,
		   which decides where it goes (thread keys are instruction << shift | that bit). */
		struct regex_program final {

				using self = ca::impl::regex_program;

				const ca::impl::regex_inst*       _insts;
				const ca::impl::regex_transition* _transitions;
				const ca::u32*                    _guards;
				const char*                       _prefix;
				ca::usz                           _prefix_size;
				ca::u32                           _size;
				ca::u32                           _start;
				ca::u32                           _slots;
				ca::u32                           _shift;
				ca::u64                           _first_bytes[4];
				bool                              _skips;
				bool                              _anchored;

		}; // struct regex_program

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_program_hpp
