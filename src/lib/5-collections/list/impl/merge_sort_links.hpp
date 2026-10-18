#ifndef ca_collections_list_impl_merge_sort_links_hpp
#define ca_collections_list_impl_merge_sort_links_hpp

#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- MERGE SORT LINKS ---------------------------------------------------

	namespace impl {

		/* @DOC The ends of a chain of links. */
		template <typename Node>
		struct link_run final {

				using self = ca::impl::link_run<Node>;

				Node* _first;
				Node* _last;

		}; // struct link_run


		/* @DOC Merges two sorted chains null-terminated by _next, left first among equals. */
		template <typename Node, typename F>
		[[nodiscard]] auto merge_link_chains(Node* left, Node* right, F& less) noexcept -> Node* {

			Node* head = nullptr;
			Node** tail = &head;
			while (left != nullptr && right != nullptr) {
				if (less(static_cast<const Node*>(right), static_cast<const Node*>(left))) {
					 *tail = right;
					 tail = &right->_next;
					right = right->_next;
				}
				else {
					*tail = left;
					 tail = &left->_next;
					 left = left->_next;
				}
			}
			*tail = left != nullptr ? left : right;
			return head;
		}

		/* @DOC Sorts the chain from first (null-terminated by _next) by less(node, node), stable, in
		   O(n log n) without allocation, then relinks _previous. Merges as a binary counter: runs of
		   1, 2, 4, ... nodes wait in 64 slots, each node merged while still in cache (libstdc++'s
		   list::sort), instead of log n sweeps of the whole list. Returns the new ends. */
		template <typename Node, typename F>
		auto merge_sort_links(Node* const first, F&& less) noexcept -> ca::impl::link_run<Node> {

			Node* pending[64] = {};
			ca::usz used = 0U;
			for (Node* at = first; at != nullptr;) {
				Node* run = at;
				        at = at->_next;
				run->_next = nullptr;
				ca::usz slot = 0U;
				for (; slot < used && pending[slot] != nullptr; ++slot) {
					          run = ca::impl::merge_link_chains(pending[slot], run, less);
					pending[slot] = nullptr;
				}
				pending[slot] = run;
				if (slot == used)
					++used;
			}
			Node* head = nullptr;
			for (ca::usz slot = 0U; slot < used; ++slot) {
				if (pending[slot] != nullptr)
					head = head == nullptr ? pending[slot] : ca::impl::merge_link_chains(pending[slot], head, less);
			}
			Node* previous = nullptr;
			for (Node* at = head; at != nullptr; at = at->_next) {
				at->_previous = previous;
				     previous = at;
			}
			return ca::impl::link_run<Node>{head, previous};
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_list_impl_merge_sort_links_hpp
