#ifndef ca_collections_regex_impl_regex_analysis_hpp
#define ca_collections_regex_impl_regex_analysis_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/regex/impl/regex_limits.hpp"
#include "5-collections/regex/impl/regex_look.hpp"
#include "5-collections/regex/impl/regex_node.hpp"
#include "5-collections/regex/impl/regex_program.hpp"
#include "5-collections/regex/impl/regex_tree.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX ANALYSIS -----------------------------------------------------

	/* @DOC What a compiled pattern tells the matcher before it runs, so it can skip text no
	   match starts in. */
	namespace impl {

		/* @DOC The bytes a match can start with, into bytes (looks taken as passing, so a
		   superset); false when a match can be empty, where every position counts. */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_first_bytes(const A& allocator, const ca::dynamic_array<ca::impl::regex_inst, A>& insts, const ca::dynamic_array<ca::impl::regex_transition, A>& transitions, const ca::u32 start, ca::u64 (&bytes)[4]) noexcept
		-> ca::result<bool> {

			for (ca::u64& word : bytes)
				word = 0U;
			ca::dynamic_array<ca::u8, A> seen{allocator};
			ca::dynamic_array<ca::u32, A> pending{allocator};
			auto ready = seen.resize(insts.size(), static_cast<ca::u8>(0U));
			if (ready)
				ready = pending.append(start);
			bool skips = true;
			while (ready && skips) {
				const ca::optional<ca::u32> popped = pending.pop();
				if (!popped)
					break;
				const ca::u32 pc = *popped;
				if (seen[pc] != 0U)
					continue;
				seen[pc] = 1U;
				const ca::impl::regex_inst& inst = insts[pc];
				switch (inst._op) {
					case ca::impl::regex_op::split:
						ready = pending.append(inst._y);
						if (ready)
							ready = pending.append(inst._x);
						break;
					case ca::impl::regex_op::save:
					case ca::impl::regex_op::look:
						ready = pending.append(inst._x);
						break;
					case ca::impl::regex_op::progress:
						ready = pending.append(inst._z);
						if (ready)
							ready = pending.append(inst._x);
						break;
					case ca::impl::regex_op::bytes:
						for (ca::u32 i = 0U; i < inst._y; ++i) {
							const ca::impl::regex_transition& transition = transitions[inst._x + i];
							for (ca::u32 value = transition._low; value <= transition._high; ++value)
								bytes[value >> 6U] |= 1ULL << (value & 63U);
						}
						break;
					case ca::impl::regex_op::match:
						skips = false;
						break;
				}
			}
			static_cast<void>(seen.cleanup());
			static_cast<void>(pending.cleanup());
			if (!ready)
				return ready.propagate();
			return skips;
		}

		/* @DOC Whether every match starts at the text's start (`\A`, or `^` outside multiline,
		   first). */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_anchored(const ca::dynamic_array<ca::impl::regex_inst, A>& insts, ca::u32 pc) noexcept -> bool {

			while (insts[pc]._op == ca::impl::regex_op::save)
				pc = insts[pc]._x;
			return insts[pc]._op == ca::impl::regex_op::look && insts[pc]._look == static_cast<ca::u8>(ca::impl::regex_look::begin_text);
		}

		/* @DOC Appends the literal text every match of node starts with to out; true when the
		   whole node was that text (no group, look or other construct), false otherwise or
		   when out ran out of memory (ok then false). */
		template <ca::is_allocator A>
		[[nodiscard]] auto regex_prefix(const ca::impl::regex_tree<A>& tree, const ca::u32 index, ca::basic_str<char, A>& out, bool& whole, bool& ok) noexcept -> bool {

			const ca::impl::regex_node& node = tree.node(index);
			switch (node._kind) {
				case ca::impl::regex_node_kind::literal:
					ok = ok && out.append_code_point(static_cast<char32_t>(node._a));
					return ok;
				case ca::impl::regex_node_kind::empty:
					return true;
				case ca::impl::regex_node_kind::look:
					whole = false;
					return true;
				case ca::impl::regex_node_kind::group:
					whole = false;
					return ca::impl::regex_prefix(tree, node._child, out, whole, ok);
				case ca::impl::regex_node_kind::concat:
					for (ca::u32 child = node._child; child != ca::impl::regex_none; child = tree.node(child)._next) {
						if (!ca::impl::regex_prefix(tree, child, out, whole, ok))
							return false;
					}
					return true;
				default:
					whole = false;
					return false;
			}
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_analysis_hpp
