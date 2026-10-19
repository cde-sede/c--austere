#ifndef ca_primitives_memory_impl_loose_bytes_hpp
#define ca_primitives_memory_impl_loose_bytes_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- LOOSE BYTES --------------------------------------------------------

	namespace impl {

		/* @DOC 16 bytes in an SSE register. */
		using bytes16 [[gnu::vector_size(16)]] = char;

		/* @DOC 32 bytes in an AVX register. */
		using bytes32 [[gnu::vector_size(32)]] = char;

		/* @DOC T at any address and of any type: an unaligned, aliasing load or store. A struct,
		   because attributes on an alias are lost when it is a template argument. */
		template <typename T>
		struct [[gnu::packed, gnu::may_alias]] loose_bytes final {
				using self = ca::impl::loose_bytes<T>;
				T _value;
		}; // struct loose_bytes

		/* @DOC T at an address aligned for T, of any type: an aliasing aligned load or store. */
		template <typename T>
		struct [[gnu::may_alias]] aligned_bytes final {
				using self = ca::impl::aligned_bytes<T>;
				T _value;
		}; // struct aligned_bytes

		/* @DOC The loose T at at, by reference: vectors never pass by value, so no ABI warning
		   where AVX is off. */
		template <typename T>
		[[gnu::always_inline]] inline auto loose(const char* const at) noexcept -> const ca::impl::loose_bytes<T>& {
			return *reinterpret_cast<const ca::impl::loose_bytes<T>*>(at);
		}

		template <typename T>
		[[gnu::always_inline]] inline auto loose(char* const at) noexcept -> ca::impl::loose_bytes<T>& {
			return *reinterpret_cast<ca::impl::loose_bytes<T>*>(at);
		}

	} // namespace impl

} // namespace ca

#endif // ca_primitives_memory_impl_loose_bytes_hpp
