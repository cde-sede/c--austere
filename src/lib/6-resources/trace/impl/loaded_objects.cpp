#include "6-resources/trace/impl/loaded_objects.hpp"


namespace ca {


	namespace trace {


		namespace impl {

			constinit ca::atomic<ca::trace::impl::loaded_object_walker> loaded_objects{nullptr};

		} // namespace impl

	} // namespace trace

} // namespace ca
