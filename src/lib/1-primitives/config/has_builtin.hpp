#ifndef ca_primitives_config_has_builtin_hpp
#define ca_primitives_config_has_builtin_hpp


// -- HAS BUILTIN ------------------------------------------------------------

/* @DOC `ca_has_builtin(x)`: `__has_builtin(x)`, or 0 where unsupported. */
#if defined(__has_builtin)
#	define ca_has_builtin(x) __has_builtin(x)
#else
#	define ca_has_builtin(x) 0
#endif

#endif // ca_primitives_config_has_builtin_hpp
