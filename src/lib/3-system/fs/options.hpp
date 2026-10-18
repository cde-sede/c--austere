#ifndef ca_system_fs_options_hpp
#define ca_system_fs_options_hpp

#include "1-primitives/config/platform.hpp"
#include "2-vocabulary/is_option.hpp"
#include "2-vocabulary/option_set.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FS -----------------------------------------------------------------

	/* @DOC Files, paths and descriptors. */
	namespace fs {


		namespace impl {

#if ca_platform(linux)
			inline constexpr int CREATE    = 0100;
			inline constexpr int EXCLUSIVE = 0200;
			inline constexpr int TRUNCATE  = 01000;
			inline constexpr int APPEND    = 02000;
			inline constexpr int NONBLOCK  = 04000;
			inline constexpr int SYNC      = 04010000;
			inline constexpr int CLOEXEC   = 02000000;
#	if defined(__aarch64__) || defined(__arm__) || defined(__powerpc__)
			inline constexpr int DIRECTORY = 040000;
			inline constexpr int NOFOLLOW  = 0100000;
#	else
			inline constexpr int DIRECTORY = 0200000;
			inline constexpr int NOFOLLOW  = 0400000;
#	endif
#else
			inline constexpr int CREATE    = 0x200;
			inline constexpr int EXCLUSIVE = 0x800;
			inline constexpr int TRUNCATE  = 0x400;
			inline constexpr int APPEND    = 0x8;
			inline constexpr int NONBLOCK  = 0x4;
			inline constexpr int SYNC      = 0x80;
			inline constexpr int CLOEXEC   = 0x1000000;
			inline constexpr int DIRECTORY = 0x100000;
			inline constexpr int NOFOLLOW  = 0x100;
#endif

		} // namespace impl


		// -- DOMAINS --------------------------------------------------------

		/* @DOC Domain of the access mode, fixed at open: readable, writable or both. */
		class access final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::access;


			public:

				// -- public constants ---------------------------------------

				static constexpr int READ_ONLY  = 0;
				static constexpr int WRITE_ONLY = 1;
				static constexpr int READ_WRITE = 2;

		}; // class access


		/* @DOC Domain of open() flags that cannot change afterwards. */
		class open_flags final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::open_flags;


			public:
		}; // class open_flags


		/* @DOC Domain of FD_* flags (F_GETFD/F_SETFD): per descriptor, not shared by duplicates. */
		class descriptor_flags final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::descriptor_flags;


			public:

				// -- public constants ---------------------------------------

				static constexpr int GET = 1;
				static constexpr int SET = 2;

		}; // class descriptor_flags


		/* @DOC Domain of file status flags (F_GETFL/F_SETFL): per open file description, shared by
		   duplicates. */
		class status_flags final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::status_flags;


			public:

				// -- public constants ---------------------------------------

				static constexpr int GET = 3;
				static constexpr int SET = 4;

		}; // class status_flags


		/* @DOC Options that fcntl can change after open: their domain names a GET/SET pair. */
		template <typename O>
		concept is_fcntl_option = ca::is_option<O> && requires {
			static_cast<int>(O::domain::GET);
			static_cast<int>(O::domain::SET);
		};


		// -- OPTIONS --------------------------------------------------------

		/* @DOC Open for reading; the default when neither readable nor writable is given. */
		class readable_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::readable_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::access;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = 1;
				static constexpr int OPEN_VALUE = 0;
				static constexpr bool SUPPORTED = true;

		}; // class readable_t


		/* @DOC Open for writing; with readable, for both. */
		class writable_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::writable_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::access;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = 2;
				static constexpr int OPEN_VALUE = 0;
				static constexpr bool SUPPORTED = true;

		}; // class writable_t


		/* @DOC O_CREAT: create the file when missing, with open's mode. */
		class create_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::create_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::open_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::CREATE;
				static constexpr int OPEN_VALUE = ca::fs::impl::CREATE;
				static constexpr bool SUPPORTED = true;

		}; // class create_t


		/* @DOC O_EXCL: with create, fail with errc::exists when the file is there. */
		class exclusive_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::exclusive_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::open_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::EXCLUSIVE;
				static constexpr int OPEN_VALUE = ca::fs::impl::EXCLUSIVE;
				static constexpr bool SUPPORTED = true;

		}; // class exclusive_t


		/* @DOC O_TRUNC: empty a regular file opened for writing. */
		class truncate_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::truncate_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::open_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::TRUNCATE;
				static constexpr int OPEN_VALUE = ca::fs::impl::TRUNCATE;
				static constexpr bool SUPPORTED = true;

		}; // class truncate_t


		/* @DOC O_DIRECTORY: fail with errc::not_a_directory unless the path is one. */
		class directory_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::directory_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::open_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::DIRECTORY;
				static constexpr int OPEN_VALUE = ca::fs::impl::DIRECTORY;
				static constexpr bool SUPPORTED = true;

		}; // class directory_t


		/* @DOC O_NOFOLLOW: fail with errc::symlink_loop when the last component is a symlink. */
		class nofollow_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::nofollow_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::open_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::NOFOLLOW;
				static constexpr int OPEN_VALUE = ca::fs::impl::NOFOLLOW;
				static constexpr bool SUPPORTED = true;

		}; // class nofollow_t


		/* @DOC O_SYNC: each write returns once data and metadata reach the device. */
		class sync_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::sync_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::open_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::SYNC;
				static constexpr int OPEN_VALUE = ca::fs::impl::SYNC;
				static constexpr bool SUPPORTED = true;

		}; // class sync_t


		/* @DOC O_APPEND: every write goes to the end of the file. */
		class append_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::append_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::status_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::APPEND;
				static constexpr int OPEN_VALUE = ca::fs::impl::APPEND;
				static constexpr bool SUPPORTED = true;

		}; // class append_t


		/* @DOC O_NONBLOCK: reads and writes fail with errc::try_again instead of waiting. */
		class nonblock_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::nonblock_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::status_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = ca::fs::impl::NONBLOCK;
				static constexpr int OPEN_VALUE = ca::fs::impl::NONBLOCK;
				static constexpr bool SUPPORTED = true;

		}; // class nonblock_t


		/* @DOC FD_CLOEXEC / O_CLOEXEC: closed across exec. */
		class cloexec_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::cloexec_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::descriptor_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = 1;
				static constexpr int OPEN_VALUE = ca::fs::impl::CLOEXEC;
				static constexpr bool SUPPORTED = true;

		}; // class cloexec_t


		/* @DOC FD_CLOFORK / O_CLOFORK: closed in fork children. SUPPORTED where the platform has it. */
		class clofork_t final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::clofork_t;


			public:

				// -- public types -------------------------------------------

				using domain = ca::fs::descriptor_flags;


				// -- public constants ---------------------------------------

				static constexpr int VALUE      = 0;
				static constexpr int OPEN_VALUE = 0;
				static constexpr bool SUPPORTED = false;

		}; // class clofork_t


		// -- OPTION CONSTANTS -----------------------------------------------

		inline constexpr ca::option_set<ca::fs::readable_t> readable{};
		inline constexpr ca::option_set<ca::fs::writable_t> writable{};
		inline constexpr ca::option_set<ca::fs::create_t> create{};
		inline constexpr ca::option_set<ca::fs::exclusive_t> exclusive{};
		inline constexpr ca::option_set<ca::fs::truncate_t> truncate{};
		inline constexpr ca::option_set<ca::fs::directory_t> directory{};
		inline constexpr ca::option_set<ca::fs::nofollow_t> nofollow{};
		inline constexpr ca::option_set<ca::fs::sync_t> sync{};
		inline constexpr ca::option_set<ca::fs::append_t> append{};
		inline constexpr ca::option_set<ca::fs::nonblock_t> nonblock{};
		inline constexpr ca::option_set<ca::fs::cloexec_t> cloexec{};
		inline constexpr ca::option_set<ca::fs::clofork_t> clofork{};

		/* @DOC Whether clofork is SUPPORTED: set() and open() report errc::not_supported otherwise. */
		inline constexpr bool has_clofork = ca::fs::clofork_t::SUPPORTED;

	} // namespace fs

} // namespace ca

#endif // ca_system_fs_options_hpp
