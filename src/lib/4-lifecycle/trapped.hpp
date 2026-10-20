#ifndef ca_lifecycle_trapped_hpp
#define ca_lifecycle_trapped_hpp

#include "1-primitives/debug/recovery.hpp"
#include "1-primitives/debug/source_location.hpp"
#include "1-primitives/utility/type_key.hpp"
#include "2-vocabulary/error.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TRAPPED ------------------------------------------------------------

	/* @DOC A trap as a context's handles sees it, at the trap site, before anything unwinds: its
	   reason, where it was (line 0 when unknown), the ca::error of the failed result accessed (or
	   of ca::ask), the error object itself by type (as<E>, nullptr for another type or for a
	   ca::error, which error() gives), and whether it can resume (ca::ask). Valid during handles
	   only. */
	class trapped final {


		private:

			// -- private types ----------------------------------------------

			using self = ca::trapped;


			// -- private members --------------------------------------------

			const ca::impl::trap_info* _info;


		public:

			// -- public lifecycle -------------------------------------------

			constexpr explicit trapped(const ca::impl::trap_info& info) noexcept
			: _info{&info} {
			}


			// -- accessors --------------------------------------------------

			[[nodiscard]] constexpr auto reason(void) const noexcept -> ca::sv::str {
				return _info->_reason == nullptr ? ca::sv::str{} : ca::sv::str{_info->_reason};
			}

			[[nodiscard]] constexpr auto where(void) const noexcept -> ca::source_location {
				return _info->_where;
			}

			[[nodiscard]] constexpr auto error(void) const noexcept -> ca::optional<ca::error> {

				if (_info->_code == 0)
					return ca::optional<ca::error>{};
				return ca::optional<ca::error>{ca::error{_info->_code, _info->_trace}};
			}

			template <typename E>
			[[nodiscard]] constexpr auto as(void) const noexcept -> const E* {

				if (_info->_type != ca::impl::type_key<E>)
					return nullptr;
				return static_cast<const E*>(_info->_payload);
			}

			[[nodiscard]] constexpr auto resumable(void) const noexcept -> bool {
				return _info->_resumable;
			}

	}; // class trapped

} // namespace ca

#endif // ca_lifecycle_trapped_hpp
