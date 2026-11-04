#ifndef ca_collections_regex_impl_regex_node_hpp
#define ca_collections_regex_impl_regex_node_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u8.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX NODE ---------------------------------------------------------

	namespace impl {

		enum class regex_node_kind : ca::u8 {
			empty,
			literal,
			set,
			look,
			repeat,
			group,
			concat,
			alternate,
		};

		/* @DOC One node of a parsed pattern, linked by index: child is the first child (concat,
		   alternate) or the only one (repeat, group), next the following sibling. a and b: a
		   literal's code point; a set's first range and range count; a look's kind; a repeat's
		   min and max; a group's capture index. */
		struct regex_node final {

				using self = ca::impl::regex_node;

				ca::u32                   _child;
				ca::u32                   _next;
				ca::u32                   _a;
				ca::u32                   _b;
				ca::impl::regex_node_kind _kind;
				bool                      _greedy;

		}; // struct regex_node

	} // namespace impl

} // namespace ca

#endif // ca_collections_regex_impl_regex_node_hpp
