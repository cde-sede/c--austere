#ifndef ca_primitives_bit_endian_hpp
#define ca_primitives_bit_endian_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- ENDIAN -------------------------------------------------------------

	/* @DOC Byte order: little, big, native. */
	enum class endian : int {
		little = __ORDER_LITTLE_ENDIAN__,
		big    = __ORDER_BIG_ENDIAN__,
		native = __BYTE_ORDER__,
	};

} // namespace ca

#endif // ca_primitives_bit_endian_hpp
