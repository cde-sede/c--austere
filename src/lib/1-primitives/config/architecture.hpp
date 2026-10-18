#ifndef ca_primitives_config_architecture_hpp
#define ca_primitives_config_architecture_hpp


// -- ARCHITECTURE -----------------------------------------------------------

/* @DOC 1 if the target CPU is `arch` (`x86_64`, `aarch64`, `riscv64`), else 0. Other CPUs build,
   with every one 0. */
#define ca_architecture(arch) (ca_architecture_impl_##arch())

#if defined(__x86_64__)
#	define ca_architecture_impl_x86_64() 1
#else
#	define ca_architecture_impl_x86_64() 0
#endif

#if defined(__aarch64__)
#	define ca_architecture_impl_aarch64() 1
#else
#	define ca_architecture_impl_aarch64() 0
#endif

#if defined(__riscv) && __riscv_xlen == 64
#	define ca_architecture_impl_riscv64() 1
#else
#	define ca_architecture_impl_riscv64() 0
#endif

#endif // ca_primitives_config_architecture_hpp
