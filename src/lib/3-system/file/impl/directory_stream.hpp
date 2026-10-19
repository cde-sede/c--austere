#ifndef ca_system_file_impl_directory_stream_hpp
#define ca_system_file_impl_directory_stream_hpp

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "3-system/file/directory_stream.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- DIRECTORY STREAM -----------------------------------------------

#if ca_platform(linux) && ca_architecture(x86_64)

		/* @DOC On linux x86-64, without libc: the directory's descriptor and a buffer of raw
		   entries (struct linux_dirent64) that getdents64 fills, [_at, _end) not yet returned.
		   The stream is one anonymous mapping of SIZE bytes. */
		class directory_stream final {
			public:
				// -- public types -------------------------------------------

				using self = ca::sys::directory_stream;

				// -- public constants ---------------------------------------

				static constexpr ca::usz SIZE = 16384U;

				// -- public members -----------------------------------------

				int _fd;
				ca::u32 _at;
				ca::u32 _end;
				alignas(8) char _entries[SIZE - 16U];
		}; // class directory_stream

		static_assert(sizeof(ca::sys::directory_stream) == ca::sys::directory_stream::SIZE);

#endif

	} // namespace sys

} // namespace ca

#endif // ca_system_file_impl_directory_stream_hpp
