#ifndef ca_collections_fs_impl_read_to_end_hpp
#define ca_collections_fs_impl_read_to_end_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/file_kind.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/file/read.hpp"
#include "5-collections/array/dynamic_array.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FS -----------------------------------------------------------------

	namespace fs {


		// -- READ TO END ----------------------------------------------------

		namespace impl {

			inline constexpr ca::usz READ_CHUNK = 4096U;

			template <typename T, typename A>
			[[nodiscard]] auto read_to_end(const int fd, ca::dynamic_array<T, A>& into) noexcept -> ca::result<void> {

				static_assert(sizeof(T) == 1U, "ca::fs::impl::read_to_end: byte-sized elements only");
				auto status = ca::sys::fstat(fd);
				const ca::usz hint = status && status->kind() == ca::sys::file_kind::regular ? static_cast<ca::usz>(status->size()) : 0U;
				ca::usz used = into.size();
				ca::usz want = used + (hint != 0U ? hint + 1U : ca::fs::impl::READ_CHUNK);
				for (;;) {
					if (used == into.size()) {
						auto grown = into.resize(want);
						if (!grown) [[unlikely]]
							return grown;
						if (ca::mul_overflow(want, into.size(), 2U)) [[unlikely]]
							return ca::failure{ca::errc::out_of_memory};
					}
					auto got = ca::sys::read(fd, ca::span{into.data(), into.size()}.without_prefix(used));
					if (!got) [[unlikely]] {
						static_cast<void>(into.resize(used));
						return got.propagate();
					}
					if (*got == 0U)
						break;
					used += *got;
				}
				return into.resize(used);
			}

		} // namespace impl

	} // namespace fs

} // namespace ca

#endif // ca_collections_fs_impl_read_to_end_hpp
