#ifndef ca_vocabulary_is_realizable_hpp
#define ca_vocabulary_is_realizable_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS REALIZABLE ------------------------------------------------------

	/* @DOC Types following the uncertain protocol. */
	template <typename U>
	concept is_realizable = requires(U& u) {
		u.holds();
		u.propagate();
	};

} // namespace ca

#endif // ca_vocabulary_is_realizable_hpp
