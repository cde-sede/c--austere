#include "3-system/error/errc.hpp"
#include "3-system/error/error_name.hpp"


[[nodiscard]] auto ca::sys::error_name(const ca::error error) noexcept -> ca::sv::str {
	switch (error.code()) {
		case ca::errc::not_permitted.code():    return "not_permitted";
		case ca::errc::no_entry.code():         return "no_entry";
		case ca::errc::interrupted.code():      return "interrupted";
		case ca::errc::bad_descriptor.code():   return "bad_descriptor";
		case ca::errc::out_of_memory.code():    return "out_of_memory";
		case ca::errc::bad_address.code():      return "bad_address";
		case ca::errc::access_denied.code():    return "access_denied";
		case ca::errc::exists.code():           return "exists";
		case ca::errc::not_a_directory.code():  return "not_a_directory";
		case ca::errc::is_a_directory.code():   return "is_a_directory";
		case ca::errc::invalid_argument.code(): return "invalid_argument";
		case ca::errc::no_space.code():         return "no_space";
		case ca::errc::out_of_range.code():     return "out_of_range";
		case ca::errc::try_again.code():        return "try_again";
		case ca::errc::name_too_long.code():    return "name_too_long";
		case ca::errc::not_empty.code():        return "not_empty";
		case ca::errc::symlink_loop.code():     return "symlink_loop";
		case ca::errc::not_supported.code():    return "not_supported";
		case ca::errc::not_implemented.code():  return "not_implemented";
		case ca::errc::not_recoverable.code():  return "not_recoverable";
		default:                                return {};
	}
}
