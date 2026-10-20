#include "3-system/error/errc.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/memory/mmap_file.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/memory/sharing.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/trace/symbolizer.hpp"


[[nodiscard]] auto ca::trace::symbolizer::make(void) noexcept -> ca::result<ca::trace::symbolizer> {

	auto file = ca::fs::descriptor::open("/proc/self/exe");
	if (!file) [[unlikely]]
		return ca::failure{file.error()};
	auto status = ca::sys::fstat(file->native());
	if (!status) [[unlikely]] {
		static_cast<void>(file->cleanup());
		return ca::failure{status.error()};
	}
	auto mapping = ca::sys::mmap_file(file->native(), 0U, static_cast<ca::usz>(status->size()), ca::sys::protection::read, ca::sys::sharing::copy_on_write);
	static_cast<void>(file->cleanup());
	if (!mapping) [[unlikely]]
		return ca::failure{mapping.error()};
	const auto image = ca::trace::impl::read_image(ca::span<const ca::byte>{mapping->data(), mapping->size()});
	if (!image) [[unlikely]] {
		static_cast<void>(ca::sys::munmap(*mapping));
		return ca::failure{ca::errc::not_supported};
	}
	const ca::trace::impl::executable_range range = ca::trace::impl::executable();
	self out{};
	out._mapping = *mapping;
	  out._image = *image;
	   out._bias = range._bias;
	  out._begin = range._begin;
	    out._end = range._end;
	return out;
}

auto ca::trace::symbolizer::lookup(const ca::usz address, const ca::span<ca::trace::location> out) const noexcept -> ca::usz {

	if (_mapping.empty() || address < _begin || address >= _end)
		return 0U;
	return ca::trace::impl::locate(_image, address - _bias, out);
}

auto ca::trace::symbolizer::of(const ca::borrowed_slice<const ca::usz> addresses) const noexcept -> ca::trace::symbolized {
	return ca::trace::symbolized{*this, addresses};
}

[[nodiscard]] auto ca::trace::symbolizer::cleanup(void) noexcept -> ca::result<void> {

	if (_mapping.empty())
		return ca::result<void>{};
	_image = ca::trace::impl::debug_image{};
	return ca::sys::munmap(ca::exchange(_mapping, ca::span<ca::byte>{}));
}
