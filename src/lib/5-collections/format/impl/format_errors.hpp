#ifndef ca_collections_format_impl_format_errors_hpp
#define ca_collections_format_impl_format_errors_hpp


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- FORMAT ERRORS ------------------------------------------------------

	/* @DOC Never defined: a format string is parsed in a consteval constructor, where calling one
	   of these is a compile error naming the problem. */
	namespace impl {

		auto format_has_more_placeholders_than_arguments(void) noexcept -> void;

		auto format_has_fewer_placeholders_than_arguments(void) noexcept -> void;

		auto format_has_unmatched_closing_brace(void) noexcept -> void;

		auto format_placeholder_is_malformed(void) noexcept -> void;

		auto format_type_does_not_fit_the_argument(void) noexcept -> void;

		auto format_precision_does_not_fit_the_argument(void) noexcept -> void;

		auto format_custom_display_takes_no_spec(void) noexcept -> void;

		auto format_width_or_precision_is_too_large(void) noexcept -> void;

		auto format_is_too_long(void) noexcept -> void;

		auto printf_conversion_is_unknown(void) noexcept -> void;

		auto printf_star_width_or_precision_is_unsupported(void) noexcept -> void;

		auto printf_n_conversion_is_unsupported(void) noexcept -> void;

	} // namespace impl

} // namespace ca

#endif // ca_collections_format_impl_format_errors_hpp
