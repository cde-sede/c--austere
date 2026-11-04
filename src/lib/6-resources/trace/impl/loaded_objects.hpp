#ifndef ca_resources_trace_impl_loaded_objects_hpp
#define ca_resources_trace_impl_loaded_objects_hpp

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- LOADED OBJECTS -------------------------------------------------

		namespace impl {

			/* @DOC A shared object a loader mapped, as a symbolizer needs it: the file (path, and
			   device and inode to tell it was not replaced), its bias and the addresses it
			   spans. Borrowed for the visit. */
			struct loaded_object final {

					using self = ca::trace::impl::loaded_object;

					ca::sv::str _path;
					ca::u64 _device;
					ca::u64 _inode;
					ca::usz _bias;
					ca::usz _begin;
					ca::usz _end;

			}; // struct loaded_object

			using loaded_object_visitor = auto (*)(void* context, const ca::trace::impl::loaded_object& object) noexcept -> void;

			using loaded_object_walker = auto (*)(ca::trace::impl::loaded_object_visitor visit, void* context) noexcept -> void;

			/* @DOC Visits every object ca's loader holds, set by the loader on its first load;
			   null in a program that never loaded one, so the symbolizer does not pull the
			   loader in. */
			extern constinit ca::atomic<ca::trace::impl::loaded_object_walker> loaded_objects;

		} // namespace impl

	} // namespace trace

} // namespace ca

#endif // ca_resources_trace_impl_loaded_objects_hpp
