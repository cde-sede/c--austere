#ifndef ca_system_io_io_buffer_hpp
#define ca_system_io_io_buffer_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/utility/forward.hpp"
#include "2-vocabulary/span.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- IO BUFFER ------------------------------------------------------

		namespace impl {

			struct io_buffer_layout;

		} // namespace impl


		/* @DOC One run of bytes for readv and writev. Same layout as struct iovec, so arrays pass to
		   the kernel uncopied. readv writes into it: the memory must be writable there. */
		class io_buffer final {


			private:

				// -- friends ------------------------------------------------

				friend struct ca::sys::impl::io_buffer_layout;


				// -- private types ------------------------------------------

				using self = ca::sys::io_buffer;


				// -- private members ----------------------------------------

				const void* _data;
				ca::usz _size;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC The memory of bytes, or of anything a span of bytes is made from: text, char and
				   byte arrays, spans. */
				constexpr io_buffer(const ca::span<const ca::byte> bytes) noexcept
				: _data{bytes.data()}, _size{bytes.size()} {
				}

				template <typename R>
				requires ca::is_constructible<ca::span<const ca::byte>, R&&>
				constexpr io_buffer(R&& bytes) noexcept
				: self{ca::span<const ca::byte>{ca::forward<R>(bytes)}} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto data(void) const noexcept -> const void* {
					return _data;
				}

				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
					return _size;
				}

		}; // class io_buffer

	} // namespace sys

} // namespace ca

#endif // ca_system_io_io_buffer_hpp
