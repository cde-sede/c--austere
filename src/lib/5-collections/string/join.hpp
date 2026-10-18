#ifndef ca_collections_string_join_hpp
#define ca_collections_string_join_hpp

#include "1-primitives/integers/usz.hpp"
#include "1-primitives/memory/string_length.hpp"
#include "1-primitives/numeric/add_overflow.hpp"
#include "1-primitives/numeric/mul_overflow.hpp"
#include "1-primitives/text/is_character.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/result.hpp"
#include "2-vocabulary/sv/basic.hpp"
#include "3-system/error/errc.hpp"
#include "4-lifecycle/memory/heap.hpp"
#include "4-lifecycle/memory/is_allocator.hpp"
#include "5-collections/string/basic_str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- JOIN ---------------------------------------------------------------

	/* @DOC The items of a range (anything converting to sv::basic<C>: views, strings, C
	   strings, the pieces of a split) with separator between them: Python's sep.join(items).
	   `ca::join(", ", names)`. Two passes, one allocation of the exact size; with an allocator
	   first, the string draws from it. */
	template <ca::is_allocator A, ca::is_character C, typename R>
	[[nodiscard]] auto join(const A& allocator, const ca::sv::basic<C> separator, const R& items) noexcept -> ca::result<ca::basic_str<C, A>> {

		ca::usz total = 0U;
		ca::usz count = 0U;
		for (const auto& item : items) {
			const ca::sv::basic<C> piece = item;
			if (ca::add_overflow(total, total, piece.size())) [[unlikely]]
				return ca::failure{ca::errc::out_of_memory};
			++count;
		}
		ca::usz separators{0U};
		if (count != 0U && (ca::mul_overflow(separators, count - 1U, separator.size()) || ca::add_overflow(total, total, separators))) [[unlikely]]
			return ca::failure{ca::errc::out_of_memory};
		ca::basic_str<C, A> out{allocator};
		auto reserved = out.reserve(total);
		if (!reserved) [[unlikely]]
			return reserved.propagate();
		bool first = true;
		for (const auto& item : items) {
			if (!first)
				static_cast<void>(out.append(separator));
			first = false;
			static_cast<void>(out.append(ca::sv::basic<C>{item}));
		}
		return out;
	}

	template <ca::is_character C, typename R>
	[[nodiscard]] auto join(const ca::sv::basic<C> separator, const R& items) noexcept -> ca::result<ca::basic_str<C>> {
		return ca::join(ca::heap{}, separator, items);
	}

	template <ca::is_character C, typename R>
	[[nodiscard]] auto join(const C* const separator, const R& items) noexcept -> ca::result<ca::basic_str<C>> {
		return ca::join(ca::heap{}, ca::sv::basic<C>{separator, ca::string_length(separator)}, items);
	}

} // namespace ca

#endif // ca_collections_string_join_hpp
