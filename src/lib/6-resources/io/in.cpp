#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/span.hpp"
#include "6-resources/io/in.hpp"


static constexpr ca::usz in_capacity = 4096U;

static constinit char in_buffer[in_capacity]{}; // file-state: ignore



// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IN -----------------------------------------------------------------

	namespace io {


		namespace impl {

			constinit ca::io::reader in_reader{0, ca::span<char>{in_buffer, in_capacity}};

		} // namespace impl

	} // namespace io

} // namespace ca
