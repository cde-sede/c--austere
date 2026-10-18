#ifndef ca_vocabulary_is_option_hpp
#define ca_vocabulary_is_option_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- IS OPTION ----------------------------------------------------------

	/* @DOC A compile-time flag: its domain (a tag type naming where the flag lives), its bit VALUE
	   in that domain, and whether the platform SUPPORTED it. Any type meeting this joins
	   option_set algebra. */
	template <typename T>
	concept is_option = requires {
		typename T::domain;
		static_cast<int>(T::VALUE);
		static_cast<bool>(T::SUPPORTED);
	};

} // namespace ca

#endif // ca_vocabulary_is_option_hpp
