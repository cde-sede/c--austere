#include "1-primitives/numeric/mul_overflow.hpp"
#include "2-vocabulary/fail.hpp"
#include "3-system/error/last_error.hpp"
#include "3-system/memory/mallocarray.hpp"

#include <errno.h>
#include <stdlib.h>


[[nodiscard]] auto ca::sys::mallocarray(const ca::usz count, const ca::usz size) noexcept -> ca::result<void*> {

	ca::usz bytes{0U};
	if (ca::mul_overflow(bytes, count, size)) [[unlikely]]
		return ca::fail(ENOMEM);
	void* const block = ::malloc(bytes == 0U ? 1U : bytes);
	if (block == nullptr) [[unlikely]]
		return ca::failure{ca::sys::impl::libc_error()};
	return block;
}
