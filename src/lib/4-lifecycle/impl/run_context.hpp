#ifndef ca_lifecycle_impl_run_context_hpp
#define ca_lifecycle_impl_run_context_hpp

#include "1-primitives/config/compiler.hpp"
#include "1-primitives/debug/recovery.hpp"
#include "1-primitives/traits/is_constructible.hpp"
#include "1-primitives/traits/is_copy_constructible.hpp"
#include "1-primitives/traits/is_same.hpp"
#include "1-primitives/traits/is_void.hpp"
#include "1-primitives/utility/move.hpp"
#include "1-primitives/utility/type_key.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/trace/context.hpp"
#include "3-system/trace/fail.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- RUN CONTEXT --------------------------------------------------------

	namespace impl {

		/* @DOC Whether a context can fail with E: E is ca::error, or made from one by E::of(error)
		   (ca::report) or a constructor. */
		template <typename E>
		concept is_context_error = ca::is_same<E, ca::error>
		                        || requires (const ca::error error) { { E::of(error) } -> ca::is_same<E>; }
		                        || ca::is_constructible<E, ca::error>;

		template <typename E>
		[[nodiscard]] auto context_error_from(const ca::error error) noexcept -> E {

			if constexpr (ca::is_same<E, ca::error>)
				return error;
			else if constexpr (requires { { E::of(error) } -> ca::is_same<E>; })
				return E::of(error);
			else
				return E{error};
		}

		/* @DOC recovery::_accept for a context failing with E: takes an error of type E, copied, or
		   moved when E cannot be copied. */
		template <typename E>
		auto accept_context_error(void* const sink, void* const error, const void* const type) noexcept -> bool {

			if (type != ca::impl::type_key<E>)
				return false;
			ca::optional<E>& out = *static_cast<ca::optional<E>*>(sink);
			if constexpr (ca::is_copy_constructible<E>)
				out.emplace(*static_cast<const E*>(error));
			else
				out.emplace(ca::move(*static_cast<E*>(error)));
			return true;
		}

		/* @DOC Sets point, makes it the thread's innermost, runs body; true when a trap came back
		   to point. Opaque to gcc's interprocedural analysis: when body never returns normally, gcc
		   would otherwise take the landing path for the only exit and assume body's stores never
		   reach the caller. */
		template <typename F>
#if ca_compiler(gcc)
		[[gnu::noipa]]
#else
		[[gnu::noinline]]
#endif
		auto enter_recovery(ca::impl::recovery& point, F& body) noexcept -> bool {

			if (__builtin_setjmp(point._buffer) != 0)
				return true;
			ca::impl::current_recovery = &point;
			body();
			return false;
		}

		/* @DOC The ca::error a context fails with when a trap came back to point without an E: the
		   error of the failed result accessed, its trace extended with the reason, else
		   not_recoverable with a new trace; the frame where the trap was (the raise call), or
		   with no location when unknown. */
		[[gnu::cold, gnu::noinline]] inline auto trapped_error(const ca::impl::recovery& point) noexcept -> ca::error {

			const ca::sv::str reason = point._reason == nullptr ? ca::sv::str{} : ca::sv::str{point._reason};
			if (point._code != 0)
				return ca::trace::context(ca::error{point._code, point._trace}, reason, point._where).error();
			return ca::trace::fail(ca::errc::not_recoverable, reason, point._where).error();
		}

		/* @DOC Runs enter() inside a new context, then leave(error), error nullptr on success. The
		   context fails with what enter returns when it returns a failed result<void, E>, or with
		   the error of a trap reached inside it: an E as it was, else a ca::error made an E. */
		template <typename E, typename Enter, typename Leave>
		[[nodiscard]] auto run_context(Enter& enter, Leave& leave) noexcept -> ca::result<void, E> {

			static_assert(ca::impl::is_context_error<E>, "ca::context_manager: the error type must be made from a ca::error");

			ca::optional<E> error{};
			ca::impl::recovery point{};
			point._parent = ca::impl::current_recovery;
			  point._sink = &error;
			point._accept = &ca::impl::accept_context_error<E>;
			auto body = [&](void) noexcept -> void {
				if constexpr (ca::is_void<decltype(enter())>)
					enter();
				else {
					auto entered = enter();
					if (!entered)
						error.emplace(ca::move(entered).error());
				}
			};

			const bool trapped = ca::impl::enter_recovery(point, body);
			ca::impl::current_recovery = point._parent;
			if (trapped && !point._accepted) [[unlikely]]
				error.emplace(ca::impl::context_error_from<E>(ca::impl::trapped_error(point)));
			if (!error.holds()) {
				leave(static_cast<const E*>(nullptr));
				return {};
			}
			leave(&*error);
			return ca::failure{*ca::move(error)};
		}

	} // namespace impl

} // namespace ca

#endif // ca_lifecycle_impl_run_context_hpp
