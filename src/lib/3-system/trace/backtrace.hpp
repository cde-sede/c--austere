#ifndef ca_system_trace_backtrace_hpp
#define ca_system_trace_backtrace_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/borrowed_slice.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRACE --------------------------------------------------------------

	namespace trace {


		// -- BACKTRACE ------------------------------------------------------

		namespace impl {

			/* @DOC Where the running executable is mapped: load bias (0 when not
			   position-independent) and the range its segments span; all 0 when unknown. */
			struct executable_range final {

					using self = ca::trace::impl::executable_range;

					ca::usz _bias;
					ca::usz _begin;
					ca::usz _end;

			}; // struct executable_range

			[[nodiscard]] auto executable(void) noexcept -> ca::trace::impl::executable_range;

			template <typename W>
			auto put_hex(W& out, ca::usz value) noexcept -> void {

				char digits[16];
				char* at = digits + sizeof(digits);
				do {
					*--at = "0123456789abcdef"[value & 15U];
					value >>= 4U;
				} while (value != 0U);
				out.put(ca::sv::str{"0x"});
				out.put(ca::sv::str{at, static_cast<ca::usz>(digits + sizeof(digits) - at)});
			}

		} // namespace impl


		/* @DOC The calls leading to a point, innermost first: one address per frame, inside its
		   call instruction (the return address minus one), ready for addr2line. Taken by walking
		   frame pointers, each read checked by the kernel (process_vm_readv), so a broken chain
		   ends the walk instead of crashing it; at most CAPACITY frames. Code built without frame
		   pointers (-fno-omit-frame-pointer) cuts the chain short or adds wrong frames past the
		   first. Linux only; empty elsewhere. Prints with `{}`, addresses inside the executable
		   relative to its load address (`exe+0x1a2b`, as addr2line -e takes them). */
		class backtrace final {


			private:

				// -- private types ------------------------------------------

				using self = ca::trace::backtrace;


			public:

				// -- public types -------------------------------------------

				static constexpr ca::usz CAPACITY = 32U;


			private:

				// -- private members ----------------------------------------

				ca::usz _addresses[CAPACITY];
				ca::usz _size;


			public:

				// -- public lifecycle ---------------------------------------

				constexpr backtrace(void) noexcept
				: _addresses{}, _size{0U} {
				}

				/* @DOC The calls leading to the caller of capture, the caller first. */
				[[nodiscard, gnu::noinline]] static auto capture(void) noexcept -> self;

				/* @DOC The calls from the frame record at frame (a live frame's
				   ca::impl::frame_address), its return address first. */
				[[nodiscard]] static auto walk(const void* frame) noexcept -> self;


				// -- accessors ----------------------------------------------

				[[nodiscard]] constexpr auto addresses(void) const noexcept -> ca::borrowed_slice<const ca::usz> {
					return ca::borrowed_slice<const ca::usz>{_addresses, _size};
				}

				[[nodiscard]] constexpr auto size(void) const noexcept -> ca::usz {
					return _size;
				}

				[[nodiscard]] constexpr auto empty(void) const noexcept -> bool {
					return _size == 0U;
				}


				// -- modifiers ----------------------------------------------

				constexpr auto clear(void) noexcept -> void {
					_size = 0U;
				}


				// -- methods ------------------------------------------------

				/* @DOC One line per frame, each on a new line: `    #0 exe+0x1a2b`, or the absolute
				   address outside the executable. */
				template <typename W>
				auto display(W& out) const noexcept -> void {

					const ca::trace::impl::executable_range exe = ca::trace::impl::executable();
					for (ca::usz i = 0U; i < _size; ++i) {
						out.put(ca::sv::str{"\n    #"});
						self::_put_index(out, i);
						out.put(' ');
						const ca::usz address = _addresses[i];
						if (address >= exe._begin && address < exe._end) {
							out.put(ca::sv::str{"exe+"});
							ca::trace::impl::put_hex(out, address - exe._bias);
						}
						else
							ca::trace::impl::put_hex(out, address);
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

		}; // class backtrace

	} // namespace trace

} // namespace ca

#endif // ca_system_trace_backtrace_hpp
