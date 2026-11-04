#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/optional.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/fstat.hpp"
#include "3-system/memory/mmap_file.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/memory/sharing.hpp"
#include "5-collections/string/str.hpp"
#include "6-resources/fs/descriptor.hpp"
#include "6-resources/trace/impl/loaded_objects.hpp"
#include "6-resources/trace/symbolizer.hpp"


namespace {

	/* The file at path mapped and its sections read; with an identity, only when the file
	   still has that device and inode. */
	[[nodiscard]] auto map_image(const char* const path, const ca::optional<ca::trace::impl::loaded_object>& identity) noexcept -> ca::result<ca::trace::impl::mapped_image> {

		auto file = ca::fs::descriptor::open(path);
		if (!file) [[unlikely]]
			return ca::failure{file.error()};
		auto status = ca::sys::fstat(file->native());
		if (!status) [[unlikely]] {
			static_cast<void>(file->cleanup());
			return ca::failure{status.error()};
		}
		if (identity && (status->device() != identity->_device || status->inode() != identity->_inode)) {
			static_cast<void>(file->cleanup());
			return ca::failure{ca::errc::no_entry};
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
		return ca::trace::impl::mapped_image{*mapping, *image, 0U, 0U, 0U};
	}

	/* Adds the object a loader holds to the symbolizer's images, when its file still reads as
	   an ELF file; an object that does not is left out. */
	auto add_loaded(void* const context, const ca::trace::impl::loaded_object& object) noexcept -> void {

		auto& objects = *static_cast<ca::dynamic_array<ca::trace::impl::mapped_image>*>(context);
		auto path = ca::str::make(object._path);
		if (!path)
			return;
		auto mapped = map_image(path->c_str(), ca::optional<ca::trace::impl::loaded_object>{object});
		static_cast<void>(path->cleanup());
		if (!mapped)
			return;
		 mapped->_bias = object._bias;
		mapped->_begin = object._begin;
		  mapped->_end = object._end;
		if (!objects.append(*mapped))
			static_cast<void>(ca::sys::munmap(mapped->_mapping));
	}

} // namespace


[[nodiscard]] auto ca::trace::symbolizer::make(void) noexcept -> ca::result<ca::trace::symbolizer> {

	auto executable = map_image("/proc/self/exe", ca::optional<ca::trace::impl::loaded_object>{});
	if (!executable) [[unlikely]]
		return executable.propagate();
	const ca::trace::impl::executable_range range = ca::trace::impl::executable();
	self out{};
	out._mapping = executable->_mapping;
	  out._image = executable->_image;
	   out._bias = range._bias;
	  out._begin = range._begin;
	    out._end = range._end;
	const ca::trace::impl::loaded_object_walker walk = ca::trace::impl::loaded_objects.load<ca::memory_order::acquire>();
	if (walk != nullptr)
		walk(add_loaded, &out._objects);
	return out;
}

auto ca::trace::symbolizer::lookup(const ca::usz address, const ca::span<ca::trace::location> out) const noexcept -> ca::usz {

	if (_mapping.empty())
		return 0U;
	if (address >= _begin && address < _end)
		return ca::trace::impl::locate(_image, address - _bias, out);
	for (const ca::trace::impl::mapped_image& object : _objects) {
		if (address >= object._begin && address < object._end)
			return ca::trace::impl::locate(object._image, address - object._bias, out);
	}
	return 0U;
}

auto ca::trace::symbolizer::of(const ca::borrowed_slice<const ca::usz> addresses) const noexcept -> ca::trace::symbolized {
	return ca::trace::symbolized{*this, addresses};
}

[[nodiscard]] auto ca::trace::symbolizer::cleanup(void) noexcept -> ca::result<void> {

	ca::result<void> outcome{};
	for (const ca::trace::impl::mapped_image& object : _objects) {
		auto unmapped = ca::sys::munmap(object._mapping);
		if (outcome && !unmapped)
			outcome = unmapped;
	}
	static_cast<void>(_objects.cleanup());
	if (_mapping.empty())
		return outcome;
	_image = ca::trace::impl::debug_image{};
	auto unmapped = ca::sys::munmap(ca::exchange(_mapping, ca::span<ca::byte>{}));
	return outcome ? unmapped : outcome;
}
