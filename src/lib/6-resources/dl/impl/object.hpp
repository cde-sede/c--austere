#ifndef ca_resources_dl_impl_object_hpp
#define ca_resources_dl_impl_object_hpp

#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/elf/dynamic_entry.hpp"
#include "3-system/elf/image.hpp"
#include "3-system/elf/program_header.hpp"
#include "5-collections/array/dynamic_array.hpp"
#include "5-collections/string/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- DL -----------------------------------------------------------------

	namespace dl {


		// -- OBJECT ---------------------------------------------------------

		namespace impl {

			/* @DOC One shared object the loader mapped: where it came from, its program headers
			   (copied, so the image never depends on them being mapped), its image, the pages
			   it owns, its TLS module id (0 without thread-local storage), how many owners it has (libraries and the objects needing it), the
			   objects it needs, and its lookup scope: itself, then what it needs, breadth
			   first. */
			struct object final {

					using self = ca::dl::impl::object;

					ca::str _path;
					ca::dynamic_array<ca::elf::program_header> _segments;
					ca::elf::image _image;
					ca::span<ca::byte> _mapping;
					ca::u64 _device;
					ca::u64 _inode;
					ca::u64 _tls_module;
					ca::usz _references;
					ca::dynamic_array<self*> _needed;
					ca::dynamic_array<self*> _scope;
					bool _relocated;
					bool _initialized;

					/* @DOC Its DT_SONAME, empty without one. */
					[[nodiscard]] auto soname(void) const noexcept -> ca::sv::str {

						const auto offset = _image.value(ca::elf::dynamic_tags::soname);
						return offset ? _image.string(*offset) : ca::sv::str{};
					}

					/* @DOC Frees its path and arrays; the pages are the loader's to unmap. */
					[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void> {

						auto path = _path.cleanup();
						auto segments = _segments.cleanup();
						auto needed = _needed.cleanup();
						auto scope = _scope.cleanup();
						if (!path)
							return path;
						if (!segments)
							return segments;
						return needed ? scope : needed;
					}

			}; // struct object

		} // namespace impl

	} // namespace dl

} // namespace ca

#endif // ca_resources_dl_impl_object_hpp
