#ifndef ca_collections_regex_regex_matches_hpp
#define ca_collections_regex_regex_matches_hpp

#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/optional.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/str.hpp"
#include "5-collections/regex/impl/pike_vm.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- REGEX MATCHES ------------------------------------------------------

	/* @DOC The matches of a regex R (a basic_regex) in a text, one per next(): what finditer
	   returns, R::matches_type. Python's rules: non-overlapping, the search going on from each
	   match's end, an empty match never twice at one position. Borrows the regex (whose
	   scratch it uses) and the text. */
	template <typename R>
	class regex_matches final {


		private:

			// -- friends ----------------------------------------------------

			friend R;


			// -- private types ----------------------------------------------

			using self = ca::regex_matches<R>;


		private:

			// -- private members --------------------------------------------

			R*          _regex;
			ca::sv::str _text;
			ca::usz     _at;
			ca::usz     _forbid;


			// -- private lifecycle ------------------------------------------

			regex_matches(R& regex, const ca::sv::str text, const ca::usz pos) noexcept
			: _regex{&regex}, _text{text}, _at{pos > text.size() ? text.size() : pos}, _forbid{ca::impl::regex_no_position} {
			}


		public:

			// -- methods ----------------------------------------------------

			/* @DOC The next match, or nothing once they are all found. Fails only out of memory
			   (a match's groups). */
			[[nodiscard]] auto next(void) noexcept -> ca::result<ca::optional<typename R::match_type>> {

				if (!_regex->_next(_text, _at, _forbid)) {
					_at = _text.size() + 1U;
					return ca::optional<typename R::match_type>{};
				}
				return _regex->_matched(_text);
			}

	}; // class regex_matches

} // namespace ca

#endif // ca_collections_regex_regex_matches_hpp
