#ifndef ca_system_elf_hashed_name_hpp
#define ca_system_elf_hashed_name_hpp

#include "1-primitives/integers/u32.hpp"
#include "2-vocabulary/sv/str.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ELF ----------------------------------------------------------------

	namespace elf {


		// -- HASHED NAME ----------------------------------------------------

		/* @DOC A symbol name with both of its ELF hashes (DT_GNU_HASH and DT_HASH), computed
		   once so a lookup through several images hashes it once. */
		struct hashed_name final {

				using self = ca::elf::hashed_name;

				ca::sv::str _text;
				ca::u32 _gnu;
				ca::u32 _sysv;

				/* @DOC The name and its hashes. */
				[[nodiscard]] static constexpr auto of(const ca::sv::str text) noexcept -> self {

					ca::u32 gnu = 5381U;
					ca::u32 sysv = 0U;
					for (const char c : text) {
						const ca::u32 unit = static_cast<unsigned char>(c);
						 gnu = gnu * 33U + unit;
						sysv = (sysv << 4U) + unit;
						const ca::u32 high = sysv & 0xF0000000U;
						if (high != 0U)
							sysv ^= high >> 24U;
						sysv &= ~high;
					}
					return self{text, gnu, sysv};
				}

		}; // struct hashed_name

	} // namespace elf

} // namespace ca

#endif // ca_system_elf_hashed_name_hpp
