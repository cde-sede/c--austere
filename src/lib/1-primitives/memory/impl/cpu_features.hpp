#ifndef ca_primitives_memory_impl_cpu_features_hpp
#define ca_primitives_memory_impl_cpu_features_hpp

#include "1-primitives/config/architecture.hpp"
#include "1-primitives/integers/usz.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- CPU FEATURES -------------------------------------------------------

#if ca_architecture(x86_64)

	namespace impl {

		/* @DOC What the byte primitives pick their variant and thresholds from, and the runtime's
		   fma its instruction. */
		struct cpu_features final {
				using self = ca::impl::cpu_features;
				bool _avx2;
				bool _bmi2;
				bool _erms;
				bool _fma;
				ca::usz _last_level_cache;
		}; // struct cpu_features

		/* @DOC The features of this CPU, from cpuid: AVX2 and FMA only when the OS saves the YMM
		   state, the last-level cache 8 MiB when cpuid does not report it. Not cached: each call
		   runs cpuid. */
		auto read_cpu_features(void) noexcept -> ca::impl::cpu_features;

	} // namespace impl

#endif

} // namespace ca

#endif // ca_primitives_memory_impl_cpu_features_hpp
