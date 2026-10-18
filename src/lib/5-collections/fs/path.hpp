/* @DOC Performance of ca::fs::basic_path<A> (ca::fs::path: A = ca::heap).

   Layout: a pointer to a NUL-terminated block from A and a size, 16 bytes on 64-bit, plus the
   allocator when stateful (24 with arena_allocator). "." owns no
   memory (null pointer), so default construction and moved-from paths cost nothing.

   | Operation | Time | Allocation |
   |---|---|---|
   | view, c_str, is_absolute, root | O(1) | none |
   | name, stem, suffix, parent | O(n) backward scan | none: views into the path |
   | components | O(n) total, lazy | none |
   | relative_to, is_relative_to, == | O(n) memcmp | none |
   | match | O(n * m) worst case per component, no backtracking across components | none |
   | from, join, push, with_name, with_stem, with_suffix, clone | O(n) single pass | one heap block |
   | filesystem queries | one syscall | none |
   | iterdir, glob | one syscall per entry | one heap block per result, plus the array |

   Normalization happens once, when a path is built: repeated slashes and "." components drop
   out, so every accessor is a scan of an already-clean string, never a re-parse. ".." is kept
   (lexically resolving it would be wrong across symlinks), as in pathlib.

   Locality: the bytes are contiguous and NUL-terminated, so system calls take c_str() with no
   copy. Views returned by the accessors point into that block and die with it.

   Relocation and copy: trivially relocatable (TRIVIALLY_RELOCATABLE), so dynamic_array<path>
   grows by remapping or in place. The copy constructor is deleted; clone() is the explicit,
   fallible copy. Moves take two words and the allocator.

   Allocation: every path, array and buffer a path builds comes from its allocator, so an
   arena-backed path builds arena-backed paths. A normalized text shorter than the block asked
   for is trimmed through ca::reallocate, in place within a heap size class.

   Baseline std::filesystem::path (libstdc++); ca over ca::heap; operations on 4 paths:
   --BENCH bench/path.cpp
   bench/path.cpp on Intel Core i7-10750H, -O2, min of 9 runs: user cycles relative to the baseline.

   | Case | clang 18 | gcc 13 | Notes |
   |---|---|---|---|
   | from, messy input, 1e6 | 0.094x | 0.13x | std: constructor + lexically_normal |
   | from, clean input, 1e6 | 0.34x | 0.49x | std: constructor only, no normalization |
   | join of a file name, 1e6 | 0.070x | 0.081x | std: operator/ |
   | name + stem + suffix, 1e6 | 0.26x | 0.33x | |
   | parent, 1e6 | 0.042x | 0.050x | |
   | component iteration, 1e6 | 2.13x | 2.62x | libstdc++ splits components once, when constructing; ca scans on each walk |
   BENCH-- */

#ifndef ca_collections_fs_path_hpp
#define ca_collections_fs_path_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/is_trivially_relocatable.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "1-primitives/utility/move.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/environment/getenv.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/chmod.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/closedir.hpp"
#include "3-system/file/file_kind.hpp"
#include "3-system/file/file_status.hpp"
#include "3-system/file/getcwd.hpp"
#include "3-system/file/link.hpp"
#include "3-system/file/lstat.hpp"
#include "3-system/file/mkdir.hpp"
#include "3-system/file/opendir.hpp"
#include "3-system/file/readdir.hpp"
#include "3-system/file/readlink.hpp"
#include "3-system/file/realpath.hpp"
#include "3-system/file/rename.hpp"
#include "3-system/file/rmdir.hpp"
#include "3-system/file/stat.hpp"
#include "3-system/file/symlink.hpp"
#include "3-system/file/unlink.hpp"
#include "3-system/memory/free.hpp"
#include "4-lifecycle/memory/allocation.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "4-lifecycle/memory/is_stateless_allocator.hpp"
#include "4-lifecycle/memory/reallocate.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/fs/impl/path_text.hpp"
#include "5-collections/fs/impl/read_to_end.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FS -----------------------------------------------------------------

	namespace fs {


		// -- PATH COMPONENT ITERATOR ----------------------------------------

		/* @DOC Walks the components of a normalized path: "/" first when absolute, then each name. */
		class path_component_iterator final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::path_component_iterator;


				// -- private members ----------------------------------------

				const char* _at;
				const char* _end;
				ca::usz _length;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr path_component_iterator(const char* const at, const char* const end) noexcept
				: _at{at}, _end{end}, _length{self::_measure(at, end)} {
				}


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto operator*(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_at, _length};
				}

				[[nodiscard]] constexpr auto operator==(const self& other) const noexcept -> bool {
					return _at == other._at;
				}


				// -- modifiers ----------------------------------------------

				constexpr auto operator++(void) noexcept -> self& {

					_at += _length;
					if (_at != _end && *_at == '/')
						++_at;
					_length = self::_measure(_at, _end);
					return *this;
				}


			private:

				// -- private methods ----------------------------------------

				[[nodiscard]] static constexpr auto _measure(const char* const at, const char* const end) noexcept -> ca::usz {

					if (at == end)
						return 0U;
					if (*at == '/')
						return 1U;
					const char* stop = at;
					while (stop != end && *stop != '/')
						++stop;
					return static_cast<ca::usz>(stop - at);
				}

		}; // class path_component_iterator


		// -- PATH COMPONENTS ------------------------------------------------

		/* @DOC Range of a path's components, as string views into it; "." has none. */
		class path_components final {


			private:

				// -- private types ------------------------------------------

				using self = ca::fs::path_components;


				// -- private members ----------------------------------------

				ca::sv::str _text;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr explicit path_components(const ca::sv::str text) noexcept
				: _text{text} {
				}


				// -- iterators ----------------------------------------------

				[[nodiscard]] constexpr auto begin(void) const noexcept -> ca::fs::path_component_iterator {
					return ca::fs::path_component_iterator{_text.begin(), _text.end()};
				}

				[[nodiscard]] constexpr auto end(void) const noexcept -> ca::fs::path_component_iterator {
					return ca::fs::path_component_iterator{_text.end(), _text.end()};
				}

		}; // class path_components


		// -- BASIC PATH -----------------------------------------------------

		/* @DOC A POSIX path, after pathlib's Path, its text in memory from A. Kept normalized: no
		   repeated slash, no trailing slash, no "." component; ".." stays. The empty path is "."
		   and owns no memory. Lexical operations (name, parent, join, match...) never touch the
		   filesystem; the others make the system calls their names say. Building a path allocates
		   and can fail; so there is no operator/, use join. Every path, array and buffer a path
		   builds comes from its allocator. Embedded NULs are rejected (errc::invalid_argument).
		   Clean up with destroy(); the destructor is a failsafe. */
		template <ca::is_allocator A = ca::heap>
		class basic_path final {


			private:

				// -- friends ------------------------------------------------

				template <ca::is_allocator>
				friend class ca::fs::basic_path;


				// -- private types ------------------------------------------

				using self = ca::fs::basic_path<A>;


			public:

				// -- public types -------------------------------------------

				using allocator_type = A;

				/* @DOC The same path type over allocator B. */
				template <ca::is_allocator B>
				using rebind = ca::fs::basic_path<B>;


				// -- public constants ---------------------------------------

				/* @DOC A pointer, a size and the allocator, no self-reference: containers move paths
				   with memcpy when A allows it. */
				static constexpr bool TRIVIALLY_RELOCATABLE = ca::is_trivially_relocatable<A>;


			private:

				// -- private members ----------------------------------------

				char*                   _data;
				ca::usz                 _size;
				[[no_unique_address]] A _allocator;


			public:

				// -- public lifecycle ---------------------------------------

				/* @DOC ".", without allocating. */
				constexpr basic_path(void) noexcept requires ca::is_stateless_allocator<A>
				: _data{nullptr}, _size{0U}, _allocator{} {
				}

				/* @DOC ".", without allocating; what it builds comes from allocator. */
				constexpr explicit basic_path(const A& allocator) noexcept
				: _data{nullptr}, _size{0U}, _allocator{allocator} {
				}

				basic_path(const self&) = delete;

				constexpr basic_path(self&& other) noexcept
				: _data{ca::exchange(other._data, nullptr)}, _size{ca::exchange(other._size, 0U)}, _allocator{other._allocator} {
				}

				/* @DOC Failsafe for a skipped destroy(): frees silently. */
				~basic_path(void) noexcept {
					this->_release();
				}

				/* @DOC text, normalized, in memory from allocator. */
				[[nodiscard]] static auto from(const A& allocator, const ca::sv::str text) noexcept -> ca::result<self> {
					return self::_compose(allocator, ca::sv::str{}, text);
				}

				[[nodiscard]] static auto from(const ca::sv::str text) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::from(A{}, text);
				}

				/* @DOC from, for ca::make_in. */
				[[nodiscard]] static auto make_in(const A& allocator, const ca::sv::str text) noexcept -> ca::result<self> {
					return self::from(allocator, text);
				}

				/* @DOC The working directory. */
				[[nodiscard]] static auto cwd(const A& allocator) noexcept -> ca::result<self> {
					for (ca::usz size = self::_first_buffer;;) {
						auto buffer = allocator.allocate(size, 1U);
						if (!buffer)
							return buffer.propagate();
						char* const text = static_cast<char*>(buffer->data());
						auto length = ca::sys::getcwd(ca::span{text, size});
						if (length) {
							auto built = self::from(allocator, ca::sv::str{text, *length});
							allocator.deallocate(*buffer, 1U);
							return built;
						}
						allocator.deallocate(*buffer, 1U);
						if (length.error() != ca::errc::out_of_range || ca::mul_overflow(size, size, 2U))
							return length.propagate();
					}
				}

				[[nodiscard]] static auto cwd(void) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::cwd(A{});
				}

				/* @DOC $HOME; errc::no_entry when unset. */
				[[nodiscard]] static auto home(const A& allocator) noexcept -> ca::result<self> {

					const auto home = ca::sys::getenv("HOME");
					if (!home)
						return ca::failure{ca::errc::no_entry};
					return self::from(allocator, *home);
				}

				[[nodiscard]] static auto home(void) noexcept -> ca::result<self> requires ca::is_stateless_allocator<A> {
					return self::home(A{});
				}


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				/* @DOC Takes other's text and allocator; the previous text is freed. */
				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					this->_release();
					     _data = ca::exchange(other._data, nullptr);
					     _size = ca::exchange(other._size, 0U);
					_allocator = other._allocator;
					return *this;
				}


				// -- accessors ----------------------------------------------

				/* @DOC The path as text; "." for the empty path. */
				[[nodiscard]] constexpr auto view(void) const noexcept -> ca::sv::str {
					return _data == nullptr ? ca::sv::str{".", 1U} : ca::sv::str{_data, _size};
				}

				/* @DOC The path as a NUL-terminated string, for system calls. */
				[[nodiscard]] constexpr auto c_str(void) const noexcept -> const char* {
					return _data == nullptr ? "." : _data;
				}

				[[nodiscard]] constexpr auto allocator(void) const noexcept -> const A& {
					return _allocator;
				}

				[[nodiscard]] constexpr auto is_absolute(void) const noexcept -> bool {
					return _size != 0U && _data[0] == '/';
				}

				/* @DOC "/" when absolute, else empty. */
				[[nodiscard]] constexpr auto root(void) const noexcept -> ca::sv::str {
					return this->is_absolute() ? ca::sv::str{"/", 1U} : ca::sv::str{};
				}

				/* @DOC Last component; empty for "/" and ".". */
				[[nodiscard]] constexpr auto name(void) const noexcept -> ca::sv::str {

					const ca::sv::str text = this->_text();
					if (text.empty() || (_size == 1U && _data[0] == '/'))
						return ca::sv::str{};
					const auto slash = text.find_last('/');
					return slash ? text.without_prefix(*slash + 1U) : text;
				}

				/* @DOC Extension of name(), dot included: from its last dot, unless the dot leads or
				   ends the name. */
				[[nodiscard]] constexpr auto suffix(void) const noexcept -> ca::sv::str {

					const ca::sv::str name = this->name();
					const auto dot = name.find_last('.');
					if (!dot || *dot == 0U || *dot + 1U == name.size())
						return ca::sv::str{};
					return name.without_prefix(*dot);
				}

				/* @DOC name() without suffix(). */
				[[nodiscard]] constexpr auto stem(void) const noexcept -> ca::sv::str {
					return this->name().without_suffix(this->suffix().size());
				}

				/* @DOC The path without its last component: "a/b" -> "a", "a" -> ".", "/a" -> "/";
				   "/" and "." are their own parent. */
				[[nodiscard]] constexpr auto parent(void) const noexcept -> ca::sv::str {

					const ca::sv::str text = this->_text();
					const auto slash = text.find_last('/');
					if (!slash)
						return ca::sv::str{".", 1U};
					return text.prefix(*slash == 0U ? 1U : *slash);
				}

				/* @DOC "/" first when absolute, then each name: "/a/b" -> "/", "a", "b". */
				[[nodiscard]] constexpr auto components(void) const noexcept -> ca::fs::path_components {
					return ca::fs::path_components{this->_text()};
				}

				/* @DOC Same text, whatever the allocators. */
				template <typename B>
				[[nodiscard]] constexpr auto operator==(const ca::fs::basic_path<B>& other) const noexcept -> bool {
					return this->view() == other.view();
				}


				// -- methods ------------------------------------------------

				/* @DOC This path followed by other; an absolute other replaces it. */
				[[nodiscard]] auto join(const ca::sv::str other) const noexcept -> ca::result<self> {
					return self::_compose(_allocator, this->_text(), other);
				}

				template <typename B>
				[[nodiscard]] auto join(const ca::fs::basic_path<B>& other) const noexcept -> ca::result<self> {
					return self::_compose(_allocator, this->_text(), other._text());
				}

				/* @DOC join, in place. On failure nothing changes. */
				[[nodiscard]] auto push(const ca::sv::str other) noexcept -> ca::result<void> {

					auto joined = this->join(other);
					if (!joined)
						return joined.propagate();
					*this = ca::move(*joined);
					return ca::result<void>{};
				}

				/* @DOC Same parent, name replaced. errc::invalid_argument when this has no name, or
				   name is empty, ".", or holds a '/' or NUL. */
				[[nodiscard]] auto with_name(const ca::sv::str name) const noexcept -> ca::result<self> {

					const ca::sv::str old = this->name();
					if (old.empty() || !ca::fs::impl::path_is_valid_name(name))
						return ca::failure{ca::errc::invalid_argument};
					return self::_concat(_allocator, this->_text().without_suffix(old.size()), name, ca::sv::str{});
				}

				/* @DOC Same parent and suffix, stem replaced. Same rules as with_name. */
				[[nodiscard]] auto with_stem(const ca::sv::str stem) const noexcept -> ca::result<self> {

					const ca::sv::str old = this->name();
					if (old.empty() || !ca::fs::impl::path_is_valid_name(stem))
						return ca::failure{ca::errc::invalid_argument};
					return self::_concat(_allocator, this->_text().without_suffix(old.size()), stem, this->suffix());
				}

				/* @DOC Same parent and stem, suffix replaced; empty removes it. errc::invalid_argument
				   when this has no name, or suffix is not empty and not '.' plus at least one char,
				   or holds a '/' or NUL. */
				[[nodiscard]] auto with_suffix(const ca::sv::str suffix) const noexcept -> ca::result<self> {

					const ca::sv::str old = this->name();
					const bool valid = suffix.empty() || (suffix.size() >= 2U && suffix.starts_with(ca::sv::str{"."})
						&& !suffix.contains('/') && !suffix.contains('\0'));
					if (old.empty() || !valid)
						return ca::failure{ca::errc::invalid_argument};
					return self::_concat(_allocator, this->_text().without_suffix(old.size()), this->stem(), suffix);
				}

				/* @DOC The rest of this path below base, lexically: "/a/b" below "/a" is "b"; "." when
				   equal; nothing when base is not a prefix of whole components. */
				template <typename B>
				[[nodiscard]] auto relative_to(const ca::fs::basic_path<B>& base) const noexcept -> ca::optional<ca::sv::str> {
					return ca::fs::impl::path_relative_to(this->_text(), base._text());
				}

				template <typename B>
				[[nodiscard]] auto is_relative_to(const ca::fs::basic_path<B>& base) const noexcept -> bool {
					return this->relative_to(base).has_value();
				}

				/* @DOC Whether the path ends with components matching pattern's, each a glob (*, ?,
				   [abc], [a-z], [!x]); an absolute pattern must match the whole path. */
				[[nodiscard]] auto match(const ca::sv::str pattern) const noexcept -> bool {
					return ca::fs::impl::path_match(this->_text(), pattern);
				}

				/* @DOC A copy, in memory from the same allocator. */
				[[nodiscard]] auto clone(void) const noexcept -> ca::result<self> {
					return self::_concat(_allocator, this->_text(), ca::sv::str{}, ca::sv::str{});
				}

				/* @DOC Frees the text, leaving ".". Never fails. */
				[[nodiscard]] auto destroy(void) noexcept -> ca::result<void> {

					this->_release();
					_data = nullptr;
					_size = 0U;
					return ca::result<void>{};
				}

				/* @DOC This path made absolute against cwd(), lexically: no symlink resolution. */
				[[nodiscard]] auto absolute(void) const noexcept -> ca::result<self> {

					if (this->is_absolute())
						return this->clone();
					auto base = self::cwd(_allocator);
					if (!base)
						return base.propagate();
					auto joined = base->join(*this);
					static_cast<void>(base->destroy());
					return joined;
				}

				/* @DOC Canonical absolute path: symlinks, "." and ".." resolved. The file must
				   exist. */
				[[nodiscard]] auto resolve(void) const noexcept -> ca::result<self> {

					auto resolved = ca::sys::realpath(this->c_str());
					if (!resolved)
						return resolved.propagate();
					auto built = self::from(_allocator, ca::sv::str{*resolved});
					ca::sys::free(*resolved);
					return built;
				}

				/* @DOC A leading "~" replaced by home(); other paths copied. */
				[[nodiscard]] auto expand_user(void) const noexcept -> ca::result<self> {

					const ca::sv::str text = this->_text();
					if (text != ca::sv::str{"~"} && !text.starts_with(ca::sv::str{"~/"}))
						return this->clone();
					auto home = self::home(_allocator);
					if (!home)
						return home.propagate();
					auto joined = home->join(text.without_prefix(2U));
					static_cast<void>(home->destroy());
					return joined;
				}

				/* @DOC stat(2): status of the file, following symlinks. */
				[[nodiscard]] auto stat(void) const noexcept -> ca::result<ca::sys::file_status> {
					return ca::sys::stat(this->c_str());
				}

				/* @DOC lstat(2): status of the file; a symlink describes itself. */
				[[nodiscard]] auto lstat(void) const noexcept -> ca::result<ca::sys::file_status> {
					return ca::sys::lstat(this->c_str());
				}

				/* @DOC Whether the path names an existing file, following symlinks. A missing
				   component, a non-directory in the middle, or a symlink loop is false; other
				   errors (errc::access_denied...) are errors. */
				[[nodiscard]] auto exists(void) const noexcept -> ca::result<bool> {
					return ca::fs::impl::path_exists(this->c_str());
				}

				/* @DOC exists() and a regular file. */
				[[nodiscard]] auto is_file(void) const noexcept -> ca::result<bool> {
					return ca::fs::impl::path_kind_is(this->c_str(), true, ca::sys::file_kind::regular);
				}

				/* @DOC exists() and a directory. */
				[[nodiscard]] auto is_directory(void) const noexcept -> ca::result<bool> {
					return ca::fs::impl::path_kind_is(this->c_str(), true, ca::sys::file_kind::directory);
				}

				/* @DOC Whether the path itself is a symlink; same error rules as exists(). */
				[[nodiscard]] auto is_symlink(void) const noexcept -> ca::result<bool> {
					return ca::fs::impl::path_kind_is(this->c_str(), false, ca::sys::file_kind::symlink);
				}

				/* @DOC Creates the directory; errc::exists when there is a file there,
				   errc::no_entry when its parent is missing. */
				[[nodiscard]] auto mkdir(const ca::u32 mode = 0777U) const noexcept -> ca::result<void> {
					return ca::sys::mkdir(this->c_str(), mode);
				}

				/* @DOC Creates the directory and its missing parents; an existing directory is
				   success. */
				[[nodiscard]] auto mkdirs(const ca::u32 mode = 0777U) const noexcept -> ca::result<void> {

					auto made = ca::sys::mkdir(this->c_str(), mode);
					if (made)
						return made;
					ca::error error = made.error();
					if (error == ca::errc::no_entry) {
						auto up = self::from(_allocator, this->parent());
						if (!up)
							return up.propagate();
						const bool top = *up == *this;
						auto parents = top ? ca::result<void>{} : up->mkdirs(mode);
						static_cast<void>(up->destroy());
						if (top)
							return made;
						if (!parents)
							return parents;
						auto again = ca::sys::mkdir(this->c_str(), mode);
						if (again)
							return again;
						error = again.error();
					}
					if (error == ca::errc::exists) {
						auto directory = this->is_directory();
						if (!directory)
							return directory.propagate();
						if (*directory)
							return ca::result<void>{};
					}
					return ca::failure{error};
				}

				/* @DOC Removes the empty directory. */
				[[nodiscard]] auto rmdir(void) const noexcept -> ca::result<void> {
					return ca::sys::rmdir(this->c_str());
				}

				/* @DOC Removes the file or symlink. */
				[[nodiscard]] auto unlink(void) const noexcept -> ca::result<void> {
					return ca::sys::unlink(this->c_str());
				}

				/* @DOC Renames the file to target, replacing a file there. */
				template <typename B>
				[[nodiscard]] auto rename(const ca::fs::basic_path<B>& target) const noexcept -> ca::result<void> {
					return ca::sys::rename(this->c_str(), target.c_str());
				}

				/* @DOC Makes this path a symlink to target. */
				template <typename B>
				[[nodiscard]] auto symlink_to(const ca::fs::basic_path<B>& target) const noexcept -> ca::result<void> {
					return ca::sys::symlink(target.c_str(), this->c_str());
				}

				/* @DOC Makes this path a hard link to the file at target. */
				template <typename B>
				[[nodiscard]] auto hardlink_to(const ca::fs::basic_path<B>& target) const noexcept -> ca::result<void> {
					return ca::sys::link(target.c_str(), this->c_str());
				}

				/* @DOC Target of this symlink, normalized. */
				[[nodiscard]] auto readlink(void) const noexcept -> ca::result<self> {
					for (ca::usz size = self::_first_buffer;;) {
						auto buffer = _allocator.allocate(size, 1U);
						if (!buffer)
							return buffer.propagate();
						char* const text = static_cast<char*>(buffer->data());
						auto length = ca::sys::readlink(this->c_str(), ca::span{text, size});
						if (!length) {
							_allocator.deallocate(*buffer, 1U);
							return length.propagate();
						}
						if (*length < size) {
							auto built = self::from(_allocator, ca::sv::str{text, *length});
							_allocator.deallocate(*buffer, 1U);
							return built;
						}
						_allocator.deallocate(*buffer, 1U);
						if (ca::mul_overflow(size, size, 2U))
							return ca::failure{ca::errc::out_of_memory};
					}
				}

				/* @DOC Sets the permission bits. */
				[[nodiscard]] auto chmod(const ca::u32 mode) const noexcept -> ca::result<void> {
					return ca::sys::chmod(this->c_str(), mode);
				}

				/* @DOC Sets the times to now, creating an empty file with mode when missing. */
				[[nodiscard]] auto touch(const ca::u32 mode = 0666U) const noexcept -> ca::result<void> {
					return ca::fs::impl::path_touch(this->c_str(), mode);
				}

				/* @DOC The whole file, in an array from the same allocator. */
				[[nodiscard]] auto read_bytes(void) const noexcept -> ca::result<ca::dynamic_array<ca::byte, A>> {
					return this->template _read<ca::byte>();
				}

				[[nodiscard]] auto read_text(void) const noexcept -> ca::result<ca::dynamic_array<char, A>> {
					return this->template _read<char>();
				}

				/* @DOC Replaces the file's contents, creating it (mode 0666, umask applies). */
				[[nodiscard]] auto write_bytes(const ca::span<const ca::byte> bytes) const noexcept -> ca::result<void> {
					return ca::fs::impl::path_write(this->c_str(), bytes);
				}

				[[nodiscard]] auto write_text(const ca::sv::str text) const noexcept -> ca::result<void> {
					return ca::fs::impl::path_write(this->c_str(), text);
				}

				/* @DOC The directory's entries, joined to this path, in directory order; paths and
				   array from the same allocator. */
				[[nodiscard]] auto iterdir(void) const noexcept -> ca::result<ca::dynamic_array<self, A>> {

					auto stream = ca::sys::opendir(this->c_str());
					if (!stream)
						return stream.propagate();
					ca::dynamic_array<self, A> entries{_allocator};
					ca::error failed{};
					for (;;) {
						auto name = ca::sys::readdir(*stream);
						if (!name) {
							failed = name.error();
							break;
						}
						if (name->empty())
							break;
						auto child = this->join(*name);
						if (!child) {
							failed = child.error();
							break;
						}
						auto added = entries.append(ca::move(*child));
						if (!added) {
							failed = added.error();
							break;
						}
					}
					auto closed = ca::sys::closedir(*stream);
					if (failed.ok() && !closed)
						failed = closed.error();
					if (!failed.ok()) {
						static_cast<void>(entries.destroy());
						return ca::failure{failed};
					}
					return entries;
				}

				/* @DOC Existing paths below this directory matching pattern, a relative path whose
				   components are globs (see match); "**" matches any number of directories,
				   symlinks not followed. Unreadable directories are skipped. Directory order. */
				[[nodiscard]] auto glob(const ca::sv::str pattern) const noexcept -> ca::result<ca::dynamic_array<self, A>> {
					return this->_globbed(pattern, false);
				}

				/* @DOC glob("**" / pattern). */
				[[nodiscard]] auto rglob(const ca::sv::str pattern) const noexcept -> ca::result<ca::dynamic_array<self, A>> {
					return this->_globbed(pattern, true);
				}


			private:

				// -- private constants --------------------------------------

				static constexpr ca::usz _first_buffer = 256U;


				// -- private methods ----------------------------------------

				constexpr basic_path(char* const data, const ca::usz size, const A& allocator) noexcept
				: _data{data}, _size{size}, _allocator{allocator} {
				}

				auto _release(void) noexcept -> void {
					if (_data != nullptr)
						_allocator.deallocate(ca::allocation{_data, _size + 1U}, 1U);
				}

				[[nodiscard]] constexpr auto _text(void) const noexcept -> ca::sv::str {
					return ca::sv::str{_data == nullptr ? "" : _data, _size};
				}

				[[nodiscard]] static auto _compose(const A& allocator, const ca::sv::str base, const ca::sv::str raw) noexcept -> ca::result<self> {

					if (raw.contains('\0'))
						return ca::failure{ca::errc::invalid_argument};
					ca::usz capacity{0U};
					if (ca::add_overflow(capacity, base.size(), raw.size(), 2U))
						return ca::failure{ca::errc::out_of_memory};
					auto block = allocator.allocate(capacity, 1U);
					if (!block)
						return block.propagate();
					char* const text = static_cast<char*>(block->data());
					ca::copy_bytes(text, base.data(), base.size());
					const ca::usz size = ca::fs::impl::path_append_components(text, base.size(), raw);
					if (size == 0U) {
						allocator.deallocate(*block, 1U);
						return self{allocator};
					}
					text[size] = '\0';
					if (size + 1U == capacity)
						return self{text, size, allocator};
					auto fitted = ca::reallocate(allocator, *block, size + 1U, 1U);
					if (!fitted) {
						allocator.deallocate(*block, 1U);
						return fitted.propagate();
					}
					return self{static_cast<char*>(fitted->data()), size, allocator};
				}

				[[nodiscard]] static auto _concat(const A& allocator, const ca::sv::str a, const ca::sv::str b, const ca::sv::str c) noexcept -> ca::result<self> {

					const ca::usz size = a.size() + b.size() + c.size();
					if (size == 0U)
						return self{allocator};
					auto block = allocator.allocate(size + 1U, 1U);
					if (!block)
						return block.propagate();
					char* const text = static_cast<char*>(block->data());
					ca::copy_bytes(text, a.data(), a.size());
					ca::copy_bytes(text + a.size(), b.data(), b.size());
					ca::copy_bytes(text + a.size() + b.size(), c.data(), c.size());
					text[size] = '\0';
					return self{text, size, allocator};
				}

				template <typename T>
				[[nodiscard]] auto _read(void) const noexcept -> ca::result<ca::dynamic_array<T, A>> {

					auto fd = ca::fs::impl::path_open_read(this->c_str());
					if (!fd)
						return fd.propagate();
					ca::dynamic_array<T, A> contents{_allocator};
					auto read = ca::fs::impl::read_to_end(*fd, contents);
					auto closed = ca::sys::close(*fd);
					if (!read || !closed) {
						static_cast<void>(contents.destroy());
						return read ? closed.propagate() : read.propagate();
					}
					return contents;
				}

				[[nodiscard]] auto _globbed(const ca::sv::str pattern, const bool deep) const noexcept -> ca::result<ca::dynamic_array<self, A>> {

					if (pattern.empty() || pattern.data()[0] == '/' || pattern.contains('\0'))
						return ca::failure{ca::errc::invalid_argument};
					ca::dynamic_array<self, A> found{_allocator};
					auto globbed = deep ? self::_glob_deep(*this, pattern, found) : self::_glob_into(*this, pattern, found);
					if (!globbed) {
						static_cast<void>(found.destroy());
						return globbed.propagate();
					}
					return found;
				}

				[[nodiscard]] static auto _glob_into(const self& base, const ca::sv::str pattern, ca::dynamic_array<self, A>& found) noexcept -> ca::result<void> {

					ca::sv::str rest = pattern;
					const ca::sv::str glob = ca::fs::impl::path_pop_first(rest);
					if (glob.empty()) {
						auto copy = base.clone();
						if (!copy)
							return copy.propagate();
						return found.append(ca::move(*copy));
					}
					if (glob == ca::sv::str{"**"})
						return self::_glob_deep(base, rest, found);
					if (!ca::fs::impl::path_has_magic(glob)) {
						auto child = base.join(glob);
						if (!child)
							return child.propagate();
						auto status = child->lstat();
						auto outcome = status ? self::_glob_into(*child, rest, found) : ca::result<void>{};
						static_cast<void>(child->destroy());
						return outcome;
					}
					auto entries = base.iterdir();
					if (!entries)
						return ca::result<void>{};
					ca::result<void> outcome{};
					for (const self& entry : *entries) {
						if (!ca::fs::impl::path_glob_matches(glob, entry.name()))
							continue;
						outcome = self::_glob_into(entry, rest, found);
						if (!outcome)
							break;
					}
					static_cast<void>(entries->destroy());
					return outcome;
				}

				[[nodiscard]] static auto _glob_deep(const self& base, const ca::sv::str rest, ca::dynamic_array<self, A>& found) noexcept -> ca::result<void> {

					auto here = self::_glob_into(base, rest, found);
					if (!here)
						return here;
					auto entries = base.iterdir();
					if (!entries)
						return ca::result<void>{};
					ca::result<void> outcome{};
					for (const self& entry : *entries) {
						auto directory = entry.lstat();
						if (!directory || directory->kind() != ca::sys::file_kind::directory)
							continue;
						outcome = self::_glob_deep(entry, rest, found);
						if (!outcome)
							break;
					}
					static_cast<void>(entries->destroy());
					return outcome;
				}

		}; // class basic_path


		// -- PATH -----------------------------------------------------------

		/* @DOC basic_path over the heap. */
		using path = ca::fs::basic_path<ca::heap>;

	} // namespace fs

} // namespace ca

#endif // ca_collections_fs_path_hpp
