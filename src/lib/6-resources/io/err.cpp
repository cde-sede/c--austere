#include "1-primitives/integers/byte.hpp"
#include "1-primitives/memory/construct_at.hpp"
#include "6-resources/io/err.hpp"



// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ERR ----------------------------------------------------------------

	namespace io {


		namespace impl {

			constinit thread_local ca::io::writer* err_current = nullptr;

		} // namespace impl

	} // namespace io

} // namespace ca


alignas(ca::io::writer) static constinit thread_local ca::byte err_slot[sizeof(ca::io::writer)]{};


[[nodiscard, gnu::cold, gnu::noinline]] auto ca::io::impl::open_err(void) noexcept -> ca::io::writer& {

	ca::io::writer* const writer = ca::construct_at(reinterpret_cast<ca::io::writer*>(err_slot), ca::io::writer::unbuffered(2));
	ca::io::impl::err_current = writer;
	return *writer;
}
