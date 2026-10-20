#ifndef ca_resources_trace_symbolizer_hpp
#define ca_resources_trace_symbolizer_hpp

#include "1-primitives/debug/trap.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/usz.hpp"
#include "1-primitives/utility/exchange.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/trace/backtrace.hpp"
#include "3-system/trace/impl/debug_info.hpp"
#include "3-system/trace/location.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- SYMBOLIZER -----------------------------------------------------

		namespace impl {

			[[noreturn, gnu::cold, gnu::noinline]] inline auto assigned_over_open_symbolizer(void) noexcept -> void {
				ca::trap("assigned over an open symbolizer");
			}

		} // namespace impl


		class symbolized;


		/* @DOC The running executable's symbols and debug information, mapped from
		   /proc/self/exe: lookup turns an address of a ca::trace::backtrace into the functions
		   it is in, innermost first (inlined ones, then the function they were inlined into),
		   each with its source file and line when the program was built with -g; of(addresses)
		   prints a whole backtrace that way with `{}`. Addresses outside the executable (shared
		   libraries) are not found. Mapped once by make(); clean up with cleanup(). */
		class symbolizer final {


			private:

				// -- private types ------------------------------------------

				using self = ca::trace::symbolizer;


				// -- private members ----------------------------------------

				ca::span<ca::byte> _mapping;
				ca::trace::impl::debug_image _image;
				ca::usz _bias;
				ca::usz _begin;
				ca::usz _end;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr symbolizer(void) noexcept
				: _mapping{}, _image{}, _bias{0U}, _begin{0U}, _end{0U} {
				}

				symbolizer(const self&) = delete;

				constexpr symbolizer(self&& other) noexcept
				: _mapping{ca::exchange(other._mapping, ca::span<ca::byte>{nullptr, 0U})}, _image{other._image}, _bias{other._bias}, _begin{other._begin}, _end{other._end} {
				}

				/* @DOC Maps the running executable and reads where its sections are. */
				[[nodiscard]] static auto make(void) noexcept -> ca::result<self>;


				// -- assignment operators -----------------------------------

				auto operator=(const self&) -> self& = delete;

				/* @DOC Takes other's mapping. Traps when this one still holds one: cleanup() first. */
				auto operator=(self&& other) noexcept -> self& {

					if (this == &other)
						return *this;
					if (!_mapping.empty()) [[unlikely]]
						ca::trace::impl::assigned_over_open_symbolizer();
					_mapping = ca::exchange(other._mapping, ca::span<ca::byte>{});
					  _image = other._image;
					   _bias = other._bias;
					  _begin = other._begin;
					    _end = other._end;
					return *this;
				}


				// -- methods ------------------------------------------------

				/* @DOC The functions address is in, innermost first, into out; how many, 0 when
				   unknown. Names, files and lines borrow from this symbolizer. */
				[[nodiscard]] auto lookup(ca::usz address, ca::span<ca::trace::location> out) const noexcept -> ca::usz;

				/* @DOC addresses (a backtrace's) symbolized, for printing with `{}`. */
				[[nodiscard]] auto of(ca::borrowed_slice<const ca::usz> addresses) const noexcept -> ca::trace::symbolized;

				/* @DOC Unmaps the executable; locations it gave become dangling. */
				[[nodiscard]] auto cleanup(void) noexcept -> ca::result<void>;

		}; // class symbolizer


		// -- SYMBOLIZED -----------------------------------------------------

		/* @DOC A backtrace as a symbolizer reads it, printed with `{}` one frame per line, inlined
		   functions first:
		   ```
		       #0 ca::trap (src/lib/1-primitives/debug/trap.hpp:23) inlined
		          deepest+0x1e (test.cpp:8)
		       #1 middle+0x9 (test.cpp:14)
		   ```
		   Borrows the symbolizer and the addresses. */
		class symbolized final {


			private:

				// -- private types ------------------------------------------

				using self = ca::trace::symbolized;


				// -- private members ----------------------------------------

				const ca::trace::symbolizer* _symbolizer;
				ca::borrowed_slice<const ca::usz> _addresses;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr symbolized(const ca::trace::symbolizer& symbolizer, const ca::borrowed_slice<const ca::usz> addresses) noexcept
				: _symbolizer{&symbolizer}, _addresses{addresses} {
				}


				// -- methods ------------------------------------------------

				template <typename W>
				auto display(W& out) const noexcept -> void {

					ca::trace::location locations[ca::trace::location::SCOPES];
					for (ca::usz i = 0U; i < _addresses.size(); ++i) {
						out.put(ca::sv::str{"    #"});
						self::_put_index(out, i);
						out.put(' ');
						const ca::usz found = _symbolizer->lookup(_addresses[i], locations);
						if (found == 0U)
							ca::trace::impl::put_hex(out, _addresses[i]);
						for (ca::usz j = 0U; j < found; ++j) {
							if (j != 0U)
								out.put(ca::sv::str{"\n       "});
							locations[j].display(out);
						}
						if (i + 1U != _addresses.size())
							out.put('\n');
					}
				}


			private:

				// -- private methods ----------------------------------------

				template <typename W>
				static auto _put_index(W& out, const ca::usz index) noexcept -> void {

					if (index >= 10U)
						out.put(static_cast<char>('0' + index / 10U));
					out.put(static_cast<char>('0' + index % 10U));
				}

		}; // class symbolized

	} // namespace trace

} // namespace ca

#endif // ca_resources_trace_symbolizer_hpp
