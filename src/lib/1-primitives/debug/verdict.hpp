#ifndef ca_primitives_debug_verdict_hpp
#define ca_primitives_debug_verdict_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- VERDICT ------------------------------------------------------------

	/* @DOC What a context's handles answers for a trap, while every frame is still alive: pass it
	   to an outer context, take it (the trap unwinds to this context, which fails with its error),
	   or resume the code that asked (ca::ask only; resuming a ca::trap stops the program). */
	enum class verdict {
		pass,
		take,
		resume,
	};

} // namespace ca

#endif // ca_primitives_debug_verdict_hpp
