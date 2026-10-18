#ifndef ca_resources_fs_descriptor_hpp
#define ca_resources_fs_descriptor_hpp

#include "1-primitives/debug/trap.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/i64.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/inverted.hpp"
#include "2-vocabulary/option_set.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/dup.hpp"
#include "3-system/file/fallocate.hpp"
#include "3-system/file/fchmod.hpp"
#include "3-system/file/fchown.hpp"
#include "3-system/file/fcntl.hpp"
#include "3-system/file/fdatasync.hpp"
#include "3-system/file/file_status.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/file/fsync.hpp"
#include "3-system/file/ftruncate.hpp"
#include "3-system/file/open.hpp"
#include "3-system/file/pread.hpp"
#include "3-system/file/pwrite.hpp"
#include "3-system/file/read.hpp"
#include "3-system/file/seek.hpp"
#include "3-system/file/seek_origin.hpp"
#include "3-system/file/unchanged_id.hpp"
#include "3-system/file/write.hpp"
#include "3-system/file/write_all.hpp"
#include "3-system/fs/options.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/fs/impl/read_to_end.hpp"
#include "5-collections/fs/path.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FS -----------------------------------------------------------------

	namespace fs {


		// -- DESCRIPTOR -----------------------------------------------------

		namespace impl {

			[[noreturn, gnu::cold, gnu::noinline]] inline auto assigned_over_open_descriptor(void) noexcept -> void {
				ca::trap();
			}

			[[noreturn, gnu::cold, gnu::noinline]] inline auto descriptor_options_not_applied(void) noexcept -> void {
				ca::trap();
			}

			template <typename O>
			inline constexpr bool is_open_option = requires {
				static_cast<int>(O::OPEN_VALUE);
			};

			template <typename... Os>
			inline constexpr int open_mask = (0 | ... | (Os::SUPPORTED ? static_cast<int>(Os::OPEN_VALUE) : 0));

			template <typename... Os>
			[[nodiscard]] constexpr auto access_mode(void) noexcept -> int {

				using chosen = ca::option_set<Os...>;
				if constexpr (!chosen::template HAS<ca::fs::writable_t>)
					return ca::fs::access::READ_ONLY;
				else if constexpr (chosen::template HAS<ca::fs::readable_t>)
					return ca::fs::access::READ_WRITE;
				else
					return ca::fs::access::WRITE_ONLY;
			}

		} // namespace impl


		/* @DOC An owned POSIX file descriptor whose flags are typed options, combined at compile
		   time: `descriptor::open(p, fs::writable | fs::create | fs::cloexec)`,
		   `d |= fs::nonblock`, `d &= ~fs::append`. Each option knows its domain (open-time,
		   FD_* or file status flags); set/clear/test issue only the fcntl calls their domains
		   need and keep every unrelated bit. Contradictions (exclusive without create, truncate
		   without writable, open-time options after open) fail to compile. Close with destroy();
		   the destructor is a failsafe that closes silently. */
		class descriptor final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::descriptor;


			public:

				// -- public constants ---------------------------------------

				/* @DOC One int, no self-reference: containers move descriptors with memcpy. */
				static constexpr bool TRIVIALLY_RELOCATABLE = true;


			private:

				// -- private members ----------------------------------------

				int _fd;


			public:

				// -- public lifecycle ---------------------------------------

				descriptor(const self&) = delete;

				constexpr descriptor(self&& other) noexcept
				: _fd{ca::exchange(other._fd, -1)} {
				}

				/* @DOC Failsafe for a skipped destroy(): closes silently. */
				~descriptor(void) noexcept {
					if (_fd != -1)
						static_cast<void>(ca::sys::close(_fd));
				}

				/* @DOC Opens path with options (readable when no access option is given); mode
				   applies with create. errc::not_supported when an option is unavailable here. */
				template <typename... Os>
				[[nodiscard]] static auto open(const char* const path, const ca::option_set<Os...> options = {}, const ca::u32 mode = 0666U) noexcept
				-> ca::result<self> {

					self::_check_open(options);
					if constexpr (!ca::option_set<Os...>::SUPPORTED)
						return ca::failure{ca::errc::not_supported};
					else {
						constexpr int flags = ca::fs::impl::access_mode<Os...>() | ca::fs::impl::open_mask<Os...>;
						auto fd = ca::sys::open(path, flags, mode);
						if (!fd)
							return fd.propagate();
						return self{*fd};
					}
				}

				template <typename A, typename... Os>
				[[nodiscard]] static auto open(const ca::fs::basic_path<A>& path, const ca::option_set<Os...> options = {}, const ca::u32 mode = 0666U) noexcept
				-> ca::result<self> {
					return self::open(path.c_str(), options, mode);
				}

				/* @DOC Takes ownership of fd, which must be open or -1. */
				[[nodiscard]] static auto adopt(const int fd) noexcept -> self {
					return self{fd};
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				/* @DOC Takes other's descriptor. Traps when this one is still open: destroy() first. */
				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					if (_fd != -1) [[unlikely]]
						ca::fs::impl::assigned_over_open_descriptor();
					_fd = ca::exchange(other._fd, -1);
					return *this;
				}

				/* @DOC set(options), trapping on failure. Unsupported options fail to compile. */
				template <typename... Os>
				auto operator|=(const ca::option_set<Os...> options) noexcept -> self& {

					static_assert(ca::option_set<Os...>::SUPPORTED, "ca::fs::descriptor |=: an option is not supported here; use set() for a result");
					if (!this->set(options)) [[unlikely]]
						ca::fs::impl::descriptor_options_not_applied();
					return *this;
				}

				/* @DOC clear(options), written `d &= ~options`, trapping on failure. */
				template <typename... Os>
				auto operator&=(const ca::inverted<ca::option_set<Os...>>) noexcept -> self& {

					if (!this->clear(ca::option_set<Os...>{})) [[unlikely]]
						ca::fs::impl::descriptor_options_not_applied();
					return *this;
				}

				/* @DOC `d &= options` would clear every bit but these, unmodeled ones included: does
				   not compile. */
				template <typename... Os>
				auto operator&=(const ca::option_set<Os...>) noexcept -> self& = delete;


				// -- accessors ----------------------------------------------

				/* @DOC The raw descriptor, still owned; -1 when moved from, destroyed or released. */
				[[nodiscard]] constexpr auto native(void) const noexcept -> int {
					return _fd;
				}

				[[nodiscard]] constexpr auto is_open(void) const noexcept -> bool {
					return _fd != -1;
				}


				// -- methods ------------------------------------------------

				/* @DOC Turns options on, all or nothing (earlier domains roll back when a later one
				   fails). errc::not_supported, before any change, when one is unavailable. */
				template <typename... Os>
				[[nodiscard]] auto set(const ca::option_set<Os...> options) noexcept -> ca::result<void> {

					self::_check_fcntl(options);
					if constexpr (!ca::option_set<Os...>::SUPPORTED)
						return ca::failure{ca::errc::not_supported};
					else
						return this->_update<Os...>(true, ca::impl::option_domains<Os...>{});
				}

				/* @DOC Turns options off, all or nothing. Unavailable options are already off. */
				template <typename... Os>
				[[nodiscard]] auto clear(const ca::option_set<Os...> options) noexcept -> ca::result<void> {

					self::_check_fcntl(options);
					return this->_update<Os...>(false, ca::impl::option_domains<Os...>{});
				}

				/* @DOC Whether every option is on. Unavailable options never are. */
				template <typename... Os>
				[[nodiscard]] auto test(const ca::option_set<Os...> options) const noexcept -> ca::result<bool> {

					self::_check_fcntl(options);
					if constexpr (!ca::option_set<Os...>::SUPPORTED)
						return false;
					else
						return this->_test<Os...>(ca::impl::option_domains<Os...>{});
				}

				/* @DOC Reads up to buffer.size() bytes; 0 at end of file. */
				[[nodiscard]] auto read(const ca::span<ca::byte> buffer) const noexcept -> ca::result<ca::usz> {
					return ca::sys::read(_fd, buffer);
				}

				/* @DOC Everything from the offset to the end of file. */
				[[nodiscard]] auto read_all(void) const noexcept -> ca::result<ca::dynamic_array<ca::byte>> {

					ca::dynamic_array<ca::byte> contents;
					auto read = ca::fs::impl::read_to_end(_fd, contents);
					if (!read) {
						static_cast<void>(contents.destroy());
						return read.propagate();
					}
					return contents;
				}

				/* @DOC Writes up to all of bytes (text passes as bytes); returns how many. */
				[[nodiscard]] auto write(const ca::span<const ca::byte> bytes) const noexcept -> ca::result<ca::usz> {
					return ca::sys::write(_fd, bytes);
				}

				/* @DOC Writes all of bytes, looping over short writes. */
				[[nodiscard]] auto write_all(const ca::span<const ca::byte> bytes) const noexcept -> ca::result<void> {
					return ca::sys::write_all(_fd, bytes);
				}

				/* @DOC Reads up to buffer.size() bytes at offset, leaving the descriptor's offset alone;
				   0 past the end. */
				[[nodiscard]] auto read_at(const ca::span<ca::byte> buffer, const ca::u64 offset) const noexcept -> ca::result<ca::usz> {
					return ca::sys::pread(_fd, buffer, offset);
				}

				/* @DOC Writes up to all of bytes at offset, leaving the descriptor's offset alone;
				   returns how many. Linux ignores offset when opened with append. */
				[[nodiscard]] auto write_at(const ca::span<const ca::byte> bytes, const ca::u64 offset) const noexcept -> ca::result<ca::usz> {
					return ca::sys::pwrite(_fd, bytes, offset);
				}

				/* @DOC Moves the offset; returns the new one, from the start. */
				[[nodiscard]] auto seek(const ca::i64 offset, const ca::sys::seek_origin origin = ca::sys::seek_origin::start) const noexcept
				-> ca::result<ca::i64> {
					return ca::sys::seek(_fd, offset, origin);
				}

				/* @DOC Flushes data and metadata to the device. */
				[[nodiscard]] auto sync(void) const noexcept -> ca::result<void> {
					return ca::sys::fsync(_fd);
				}

				/* @DOC Flushes data to the device, and only the metadata needed to read it back. */
				[[nodiscard]] auto sync_data(void) const noexcept -> ca::result<void> {
					return ca::sys::fdatasync(_fd);
				}

				/* @DOC Cuts the file to size bytes, or extends it with zeros. */
				[[nodiscard]] auto resize(const ca::u64 size) const noexcept -> ca::result<void> {
					return ca::sys::ftruncate(_fd, size);
				}

				/* @DOC Reserves disk space for bytes offset..offset + size, growing the file to cover
				   them; later writes there cannot fail for lack of space. */
				[[nodiscard]] auto allocate(const ca::u64 offset, const ca::u64 size) const noexcept -> ca::result<void> {
					return ca::sys::fallocate(_fd, offset, size);
				}

				/* @DOC Sets the permission bits. */
				[[nodiscard]] auto chmod(const ca::u32 mode) const noexcept -> ca::result<void> {
					return ca::sys::fchmod(_fd, mode);
				}

				/* @DOC Sets the owner; sys::unchanged_id keeps user or group. */
				[[nodiscard]] auto chown(const ca::u32 user, const ca::u32 group = ca::sys::unchanged_id) const noexcept -> ca::result<void> {
					return ca::sys::fchown(_fd, user, group);
				}

				/* @DOC fstat(2). */
				[[nodiscard]] auto stat(void) const noexcept -> ca::result<ca::sys::file_status> {
					return ca::sys::fstat(_fd);
				}

				/* @DOC A second descriptor on the same open file: status flags (append, nonblock)
				   and offset shared; descriptor flags fresh, cloexec the only one accepted. */
				template <typename... Os>
				[[nodiscard]] auto duplicate(const ca::option_set<Os...> = {}) const noexcept -> ca::result<self> {

					static_assert(((ca::is_same<Os, ca::fs::cloexec_t>) && ...), "ca::fs::descriptor::duplicate: only cloexec applies to a duplicate");
					auto fd = ca::sys::dup(_fd, sizeof...(Os) != 0U);
					if (!fd)
						return fd.propagate();
					return self{*fd};
				}

				/* @DOC Gives up ownership; the descriptor is the caller's to close. */
				[[nodiscard]] constexpr auto release(void) noexcept -> int {
					return ca::exchange(_fd, -1);
				}

				/* @DOC Closes. The descriptor is gone even on failure; a closed one succeeds. */
				[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {

					if (_fd == -1)
						return ca::result<void>{};
					return ca::sys::close(ca::exchange(_fd, -1));
				}


			private:

				// -- private methods ----------------------------------------

				constexpr explicit descriptor(const int fd) noexcept
				: _fd{fd} {
				}

				template <typename... Os>
				static constexpr auto _check_open(ca::option_set<Os...>) noexcept -> void {

					using chosen = ca::option_set<Os...>;
					static_assert((ca::fs::impl::is_open_option<Os> && ...), "ca::fs::descriptor::open: not a ca::fs option");
					static_assert(!chosen::template HAS<ca::fs::exclusive_t> || chosen::template HAS<ca::fs::create_t>,
						"ca::fs::descriptor::open: exclusive needs create");
					static_assert(!chosen::template HAS<ca::fs::truncate_t> || chosen::template HAS<ca::fs::writable_t>,
						"ca::fs::descriptor::open: truncate needs writable");
					static_assert(!chosen::template HAS<ca::fs::directory_t>
						|| !(chosen::template HAS<ca::fs::writable_t> || chosen::template HAS<ca::fs::create_t>),
						"ca::fs::descriptor::open: a directory opens read-only, without create");
				}

				template <typename... Os>
				static constexpr auto _check_fcntl(ca::option_set<Os...>) noexcept -> void {
					static_assert((ca::fs::is_fcntl_option<Os> && ...), "ca::fs::descriptor: open-time options cannot change after open");
				}

				template <typename... Os, typename... Ds>
				[[nodiscard]] auto _update(const bool on, ca::impl::option_list<Ds...>) noexcept -> ca::result<void> {

					if constexpr (sizeof...(Ds) != 0U) {
						constexpr int gets[]  = {Ds::GET...};
						constexpr int sets[]  = {Ds::SET...};
						constexpr int masks[] = {ca::impl::option_mask<Ds, Os...>...};
						int old[sizeof...(Ds)]{};
						bool written[sizeof...(Ds)]{};
						for (ca::usz i = 0U; i < sizeof...(Ds); ++i) {
							auto current = ca::sys::fcntl(_fd, gets[i]);
							ca::error failed{};
							if (!current)
								failed = current.error();
							else {
								const int next = on ? (*current | masks[i]) : (*current & ~masks[i]);
								old[i] = *current;
								if (next != *current) {
									auto changed = ca::sys::fcntl(_fd, sets[i], next);
									if (!changed)
										failed = changed.error();
									else
										written[i] = true;
								}
							}
							if (!failed.ok()) {
								for (ca::usz j = i; j != 0U; --j) {
									if (written[j - 1U])
										static_cast<void>(ca::sys::fcntl(_fd, sets[j - 1U], old[j - 1U]));
								}
								return ca::failure{failed};
							}
						}
					}
					return ca::result<void>{};
				}

				template <typename... Os, typename... Ds>
				[[nodiscard]] auto _test(ca::impl::option_list<Ds...>) const noexcept -> ca::result<bool> {

					if constexpr (sizeof...(Ds) != 0U) {
						constexpr int gets[]  = {Ds::GET...};
						constexpr int masks[] = {ca::impl::option_mask<Ds, Os...>...};
						for (ca::usz i = 0U; i < sizeof...(Ds); ++i) {
							auto current = ca::sys::fcntl(_fd, gets[i]);
							if (!current)
								return current.propagate();
							if ((*current & masks[i]) != masks[i])
								return false;
						}
					}
					return true;
				}

		}; // class descriptor

	} // namespace fs

} // namespace ca

#endif // ca_resources_fs_descriptor_hpp
