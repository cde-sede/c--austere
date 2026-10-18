#ifndef ca_primitives_config_platform_hpp
#define ca_primitives_config_platform_hpp


// -- PLATFORM ---------------------------------------------------------------

/* @DOC 1 if the target OS is `os` (`linux`, `macos`), else 0. Other platforms fail to compile. */
#define ca_platform(os) (ca_platform_impl_##os())

#if defined(__APPLE__) || defined(__MACH__)
#	define ca_platform_impl_macos() 1
#	define ca_platform_impl_linux() 0

#elif defined(__linux__)
#	define ca_platform_impl_macos() 0
#	define ca_platform_impl_linux() 1

#else
#	error "ca: unsupported platform"
#endif

#endif // ca_primitives_config_platform_hpp
