#ifndef ca_system_trace_location_hpp
#define ca_system_trace_location_hpp

#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/backtrace.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		namespace impl {

			struct location_access;

		} // namespace impl


		// -- LOCATION -------------------------------------------------------

		/* @DOC One function an address is in, as the debug information of the executable tells
		   it: the function's name and the scopes around it (namespaces, classes, the enclosing
		   function of a local class), outermost first; the source file and line; the offset
		   from the function's start (the outermost function of an address only); whether the
		   compiler inlined it into the next location. Names come from DWARF when the program was
		   built with -g, else from the symbol table (mangled); file and line need -g. Text
		   borrows from the ca::trace::symbolizer that made it. Prints with `{}`:
		   `ca::trap (src/trap.hpp:23) inlined`, `deepest+0x1e (test.cpp:8)`. */
		class location final {


			private:

				// -- friends ------------------------------------------------

				friend struct ca::trace::impl::location_access;


				// -- private types ------------------------------------------

				using self = ca::trace::location;


			public:

				// -- public types -------------------------------------------

				static constexpr ca::usz SCOPES = 8U;


			private:

				// -- private members ----------------------------------------

				ca::sv::str _names[SCOPES];
				ca::usz _count;
				ca::sv::str _root;
				ca::sv::str _directory;
				ca::sv::str _file;
				ca::u32 _line;
				ca::usz _offset;
				bool _has_offset;
				bool _inlined;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr location(void) noexcept
				: _names{}, _count{0U}, _root{}, _directory{}, _file{}, _line{0U}, _offset{0U}, _has_offset{false}, _inlined{false} {
				}


				// -- accessors ----------------------------------------------

				/* @DOC The function's own name, empty when unknown. */
				[[nodiscard]] constexpr auto function(void) const noexcept -> ca::sv::str {
					return _count == 0U ? ca::sv::str{} : _names[_count - 1U];
				}

				/* @DOC The scopes around the function, outermost first; the innermost SCOPES - 1 when
				   deeper. */
				[[nodiscard]] constexpr auto scopes(void) const noexcept -> ca::borrowed_slice<const ca::sv::str> {
					return ca::borrowed_slice<const ca::sv::str>{_names, _count == 0U ? 0U : _count - 1U};
				}

				/* @DOC The source file, as the compiler named it: relative to directory() unless
				   absolute. Empty when unknown. */
				[[nodiscard]] constexpr auto file(void) const noexcept -> ca::sv::str {
					return _file;
				}

				/* @DOC The file's directory, relative to compilation_directory() unless absolute;
				   empty when file() is absolute. */
				[[nodiscard]] constexpr auto directory(void) const noexcept -> ca::sv::str {
					return _directory;
				}

				/* @DOC Where the compiler ran, which relative directories start from. */
				[[nodiscard]] constexpr auto compilation_directory(void) const noexcept -> ca::sv::str {
					return _root;
				}

				/* @DOC 0 when unknown. */
				[[nodiscard]] constexpr auto line(void) const noexcept -> ca::u32 {
					return _line;
				}

				/* @DOC From the function's start, for the outermost function of an address. */
				[[nodiscard]] constexpr auto offset(void) const noexcept -> ca::optional<ca::usz> {
					return _has_offset ? ca::optional<ca::usz>{_offset} : ca::optional<ca::usz>{};
				}

				/* @DOC Whether the compiler inlined this function into the next location. */
				[[nodiscard]] constexpr auto inlined(void) const noexcept -> bool {
					return _inlined;
				}


				// -- methods ------------------------------------------------

				template <typename W>
				auto display(W& out) const noexcept -> void {

					if (_count == 0U)
						out.put(ca::sv::str{"??"});
					for (ca::usz i = 0U; i < _count; ++i) {
						if (i != 0U)
							out.put(ca::sv::str{"::"});
						out.put(_names[i]);
					}
					if (_has_offset) {
						out.put('+');
						ca::trace::impl::put_hex(out, _offset);
					}
					if (!_file.empty()) {
						out.put(ca::sv::str{" ("});
						const bool absolute_file = _file.data()[0] == '/';
						const bool absolute_directory = !_directory.empty() && _directory.data()[0] == '/';
						if (!absolute_file && !absolute_directory && !_root.empty()) {
							out.put(_root);
							out.put('/');
						}
						if (!absolute_file && !_directory.empty()) {
							out.put(_directory);
							out.put('/');
						}
						out.put(_file);
						if (_line != 0U) {
							out.put(':');
							self::_put_line(out, _line);
						}
						out.put(')');
					}
					if (_inlined)
						out.put(ca::sv::str{" inlined"});
				}


			private:

				// -- private methods ----------------------------------------

				template <typename W>
				static auto _put_line(W& out, ca::u32 line) noexcept -> void {

					char digits[10];
					char* at = digits + sizeof(digits);
					do {
						*--at = static_cast<char>('0' + line % 10U);
						line /= 10U;
					} while (line != 0U);
					out.put(ca::sv::str{at, static_cast<ca::usz>(digits + sizeof(digits) - at)});
				}

		}; // class location

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_location_hpp
