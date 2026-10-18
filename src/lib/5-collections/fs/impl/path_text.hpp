#ifndef ca_collections_fs_impl_path_text_hpp
#define ca_collections_fs_impl_path_text_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/file/file_kind.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FS -----------------------------------------------------------------

	namespace fs {


		// -- PATH TEXT ------------------------------------------------------

		/* @DOC The parts of basic_path that allocate nothing, compiled once for every allocator. */
		namespace impl {

			/* @DOC Errors exists() and the kind queries report as false. */
			[[nodiscard]] auto path_is_missing(const ca::error& error) noexcept -> bool;

			/* @DOC Not empty, not ".", no '/', no NUL. */
			[[nodiscard]] auto path_is_valid_name(ca::sv::str name) noexcept -> bool;

			/* @DOC Appends raw's components to the size bytes at out, normalized; returns the new
			   size. out holds size + raw.size() + 1 bytes. */
			[[nodiscard]] auto path_append_components(char* out, ca::usz size, ca::sv::str raw) noexcept -> ca::usz;

			/* @DOC First component of rest, "." skipped; rest keeps what follows. */
			[[nodiscard]] auto path_pop_first(ca::sv::str& rest) noexcept -> ca::sv::str;

			[[nodiscard]] auto path_has_magic(ca::sv::str pattern) noexcept -> bool;

			/* @DOC Whether name matches the glob pattern (*, ?, [abc], [a-z], [!x]). */
			[[nodiscard]] auto path_glob_matches(ca::sv::str pattern, ca::sv::str name) noexcept -> bool;

			/* @DOC basic_path::match on the normalized text. */
			[[nodiscard]] auto path_match(ca::sv::str text, ca::sv::str pattern) noexcept -> bool;

			/* @DOC basic_path::relative_to on normalized texts. */
			[[nodiscard]] auto path_relative_to(ca::sv::str text, ca::sv::str base) noexcept -> ca::optional<ca::sv::str>;

			[[nodiscard]] auto path_exists(const char* path) noexcept -> ca::result<bool>;

			[[nodiscard]] auto path_kind_is(const char* path, bool follow, ca::sys::file_kind kind) noexcept -> ca::result<bool>;

			[[nodiscard]] auto path_touch(const char* path, ca::u32 mode) noexcept -> ca::result<void>;

			[[nodiscard]] auto path_write(const char* path, ca::span<const ca::byte> bytes) noexcept -> ca::result<void>;

			/* @DOC Opens path for reading, close-on-exec; the descriptor. */
			[[nodiscard]] auto path_open_read(const char* path) noexcept -> ca::result<int>;

		} // namespace impl

	} // namespace fs

} // namespace ca

#endif // ca_collections_fs_impl_path_text_hpp
