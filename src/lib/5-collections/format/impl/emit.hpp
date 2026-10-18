#ifndef ca_collections_format_impl_emit_hpp
#define ca_collections_format_impl_emit_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/io/is_writer.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/io/is_windowed_writer.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/span.hpp"
#include "5-collections/format/impl/display_argument.hpp"
#include "5-collections/format/impl/format_piece.hpp"
#include "5-collections/format/impl/format_spec.hpp"
#include "5-collections/format/impl/render.hpp"
#include "5-collections/format/text_writer.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- EMIT ---------------------------------------------------------------

	namespace impl {

		inline constexpr ca::usz stage_capacity = 1024U;

		template <typename S>
		struct staging final {

				using self = ca::impl::staging<S>;

				S* _writer;
				char* _stage;
				ca::usz _size;

				[[nodiscard]] static auto drain(void* const target, char* const at) noexcept -> ca::result<ca::span<char>> {

					self* const staged = static_cast<self*>(target);
					const ca::usz size = static_cast<ca::usz>(at - staged->_stage);
					auto written = staged->_writer->write(ca::span<const char>{staged->_stage, size});
					if (!written) [[unlikely]]
						return written.propagate();
					staged->_size += size;
					return ca::span<char>{staged->_stage, ca::impl::stage_capacity};
				}

		}; // struct staging

		template <typename S>
		struct lending final {

				using self = ca::impl::lending<S>;

				S* _writer;
				char* _start;
				ca::usz _size;

				[[nodiscard]] static auto drain(void* const target, char* const at) noexcept -> ca::result<ca::span<char>> {

					self* const lent = static_cast<self*>(target);
					auto next = lent->_writer->drain(at);
					if (!next) [[unlikely]]
						return next;
					lent->_size += static_cast<ca::usz>(at - lent->_start);
					lent->_start = next->begin();
					return next;
				}

		}; // struct lending

		/* @DOC One message to writer: formatted in place in its window when it lends one, else
		   staged on the stack and written per 1 KiB. Its size in bytes, or its first error. */
		template <ca::io::is_writer S>
		[[nodiscard]] auto emit(S& writer, const char* const text, const ca::impl::format_piece* const pieces, const ca::impl::format_spec* const specs, const ca::impl::display_argument* const arguments, const ca::usz count) noexcept -> ca::result<ca::usz> {

			if constexpr (ca::io::is_windowed_writer<S>) {
				const ca::span<char> window = writer.window();
				if (!window.empty()) [[likely]] {
					ca::impl::lending<S> lent{&writer, window.begin(), 0U};
					ca::text_writer out{window, &lent, &ca::impl::lending<S>::drain};
					ca::impl::render(out, text, pieces, specs, arguments, count);
					if (!out.error().ok()) [[unlikely]]
						return ca::failure{out.error()};
					auto settled = writer.settle(out.position());
					if (!settled) [[unlikely]]
						return settled.propagate();
					return lent._size + static_cast<ca::usz>(out.position() - lent._start);
				}
			}
			char stage[ca::impl::stage_capacity];
			ca::impl::staging<S> staged{&writer, stage, 0U};
			ca::text_writer out{ca::span<char>{stage}, &staged, &ca::impl::staging<S>::drain};
			ca::impl::render(out, text, pieces, specs, arguments, count);
			if (!out.error().ok()) [[unlikely]]
				return ca::failure{out.error()};
			const ca::usz size = static_cast<ca::usz>(out.position() - stage);
			auto written = writer.write(ca::span<const char>{stage, size});
			if (!written) [[unlikely]]
				return written.propagate();
			return staged._size + size;
		}

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_emit_hpp
