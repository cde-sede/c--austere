#ifndef ca_collections_list_list_link_hpp
#define ca_collections_list_list_link_hpp

#include "2-vocabulary/impl/access.hpp"
#include "5-collections/list/impl/merge_sort_links.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LIST LINK ----------------------------------------------------------

	namespace impl {

		template <typename U, typename Tag>
		class intrusive_list_iterator;

	} // namespace impl


	/* @DOC The two links T carries to be in an intrusive_list<T, Tag>: derive T from it,
	   publicly, once per Tag for each list an element may be in at the same time. A copy or move
	   starts unlinked; destroying a linked one traps, in every build. */
	template <typename T, typename Tag = void> // final: ignore
	class list_link {


		private:

			// -- friends ----------------------------------------------------

			template <typename, typename>
			friend class intrusive_list; // qualify: ignore

			template <typename, typename>
			friend class ca::impl::intrusive_list_iterator;

			template <typename Node, typename F>
			friend auto ca::impl::merge_sort_links(Node*, F&&) noexcept -> ca::impl::link_run<Node>;

			template <typename Node, typename F>
			friend auto ca::impl::merge_link_chains(Node*, Node*, F&) noexcept -> Node*;


			// -- private types ----------------------------------------------

			using self = ca::list_link<T, Tag>;


			// -- private members --------------------------------------------

			self* _previous;
			self* _next;


		public:

			// -- public lifecycle -------------------------------------------

			/* @DOC Unlinked. */
			constexpr list_link(void) noexcept
			: _previous{nullptr}, _next{this} {
			}

			/* @DOC Unlinked: links belong to the element's place, not its value. */
			constexpr list_link(const self&) noexcept
			: _previous{nullptr}, _next{this} {
			}

			/* @DOC Traps when still linked: unlink the element first. */
			constexpr ~list_link(void) noexcept {
				if (_next != this) [[unlikely]]
					ca::impl::element_destroyed_while_linked();
			}


			// -- assignment operators ---------------------------------------

			/* @DOC Keeps this element's links as they are. */
			constexpr auto operator=(const self&) noexcept -> self& {
				return *this;
			}


			// -- accessors --------------------------------------------------

			/* @DOC The element is in a list through this link. */
			[[nodiscard]] constexpr auto linked(void) const noexcept -> bool {
				return _next != this;
			}

	}; // class list_link

} // namespace ca

#endif // ca_collections_list_list_link_hpp
