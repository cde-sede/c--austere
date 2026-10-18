#ifndef ca_system_file_file_status_hpp
#define ca_system_file_file_status_hpp

#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "3-system/file/file_kind.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- SYS ----------------------------------------------------------------

	namespace sys {


		// -- FILE STATUS ----------------------------------------------------

		/* @DOC What stat reports about a file. */
		class file_status final {


			private:

				// -- private types ------------------------------------------

				using self = ca::sys::file_status;


				// -- private members ----------------------------------------

				ca::sys::file_kind _kind;
				ca::u32 _permissions;
				ca::u64 _size;
				ca::u64 _inode;
				ca::u64 _device;
				ca::u64 _links;
				ca::u32 _user;
				ca::u32 _group;
				ca::i64 _accessed;
				ca::i64 _modified;
				ca::i64 _changed;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr file_status(const ca::sys::file_kind kind, const ca::u32 permissions, const ca::u64 size, const ca::u64 inode, const ca::u64 device, const ca::u64 links, const ca::u32 user, const ca::u32 group, const ca::i64 accessed, const ca::i64 modified, const ca::i64 changed) noexcept
				: _kind{kind}, _permissions{permissions}, _size{size}, _inode{inode}, _device{device}, _links{links}, _user{user}, _group{group}, _accessed{accessed}, _modified{modified}, _changed{changed} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC Type of the file. */
				[[nodiscard]] constexpr auto kind(void) const noexcept -> ca::sys::file_kind {
					return _kind;
				}

				/* @DOC Permission bits (07777). */
				[[nodiscard]] constexpr auto permissions(void) const noexcept -> ca::u32 {
					return _permissions;
				}

				/* @DOC Size in bytes; symlinks: length of the target. */
				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::u64 {
					return _size;
				}

				/* @DOC Inode number. */
				[[nodiscard]] constexpr auto inode(void) const noexcept -> ca::u64 {
					return _inode;
				}

				/* @DOC Device holding the file. */
				[[nodiscard]] constexpr auto device(void) const noexcept -> ca::u64 {
					return _device;
				}

				/* @DOC Hard link count. */
				[[nodiscard]] constexpr auto links(void) const noexcept -> ca::u64 {
					return _links;
				}

				/* @DOC Owner user id. */
				[[nodiscard]] constexpr auto user(void) const noexcept -> ca::u32 {
					return _user;
				}

				/* @DOC Owner group id. */
				[[nodiscard]] constexpr auto group(void) const noexcept -> ca::u32 {
					return _group;
				}

				/* @DOC Last access, nanoseconds since the epoch. */
				[[nodiscard]] constexpr auto accessed(void) const noexcept -> ca::i64 {
					return _accessed;
				}

				/* @DOC Last content change, nanoseconds since the epoch. */
				[[nodiscard]] constexpr auto modified(void) const noexcept -> ca::i64 {
					return _modified;
				}

				/* @DOC Last status change, nanoseconds since the epoch. */
				[[nodiscard]] constexpr auto changed(void) const noexcept -> ca::i64 {
					return _changed;
				}

		}; // class file_status

	} // namespace sys

} // namespace ca

#endif // ca_system_file_file_status_hpp
