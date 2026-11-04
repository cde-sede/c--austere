#ifndef ca_resources_dl_impl_loader_hpp
#define ca_resources_dl_impl_loader_hpp

#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/trace/report.hpp"
#include "6-resources/dl/host_symbol.hpp"
#include "6-resources/dl/impl/object.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DL -----------------------------------------------------------------

	namespace dl {


		// -- LOADER ---------------------------------------------------------

		namespace impl {

			/* @DOC Loads name (a path, or a file name searched for) and what it needs, relocates
			   and initializes what is new, and returns it with one more reference. Undefined
			   symbols resolve against host first, then the running program's exported
			   symbols, then the new object's scope. All or nothing:
			   a failure leaves every object as it was. Serialized by one lock, held through the
			   initializers. */
			[[nodiscard]] auto load(ca::sv::str name, ca::borrowed_slice<const ca::dl::host_symbol> host) noexcept -> ca::result<ca::dl::impl::object*, ca::report<>>;

			/* @DOC Drops a reference to object; the last one runs its finalizers, releases what it
			   needs and unmaps it. */
			[[nodiscard]] auto release(ca::dl::impl::object* object) noexcept -> ca::result<void>;

			/* @DOC The address of the symbol named name in object's scope: an indirect function
			   resolved, a thread-local variable the calling thread's copy; none when
			   undefined. */
			[[nodiscard]] auto lookup(const ca::dl::impl::object& object, ca::sv::str name) noexcept -> ca::optional<void*>;

		} // namespace impl

	} // namespace dl

} // namespace ca

#endif // ca_resources_dl_impl_loader_hpp
