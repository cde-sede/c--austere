#ifndef ca_primitives_config_compiler_hpp
#define ca_primitives_config_compiler_hpp


// -- COMPILER ---------------------------------------------------------------

/* @DOC 1 if the compiler is `name` (`clang`, `gcc`), else 0. Other compilers fail to compile. */
#define ca_compiler(name) (ca_compiler_impl_##name())

#if defined(__clang__)
#	define ca_compiler_impl_clang() 1
#	define ca_compiler_impl_gcc() 0

#elif defined(__GNUC__)
#	define ca_compiler_impl_clang() 0
#	define ca_compiler_impl_gcc() 1

#else
#	error "ca: unsupported compiler"
#endif

#endif // ca_primitives_config_compiler_hpp
