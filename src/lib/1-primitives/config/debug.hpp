/* @DOC `ca_debug`: 1 unless NDEBUG is defined; define it to 0 or 1 to override. Enables the access
   checks of uncertain containers. */

#ifndef ca_primitives_config_debug_hpp
#define ca_primitives_config_debug_hpp


// -- DEBUG ------------------------------------------------------------------

#if !defined(ca_debug)
#	if defined(NDEBUG)
#		define ca_debug 0
#	else
#		define ca_debug 1
#	endif
#endif

#endif // ca_primitives_config_debug_hpp
