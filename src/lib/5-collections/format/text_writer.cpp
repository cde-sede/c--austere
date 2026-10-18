#include "1-primitives/integers/u8.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/fill_bytes.hpp"
#include "1-primitives/numeric/min.hpp"
#include "3-system/error/errc.hpp"
#include "5-collections/format/text_writer.hpp"


auto ca::text_writer::_refill(void) noexcept -> bool {

	if (_drain == nullptr)
		return false;
	auto window = _drain(_target, _at);
	if (!window || window->empty()) [[unlikely]] {
		_error = window ? ca::errc::no_space : window.error();
		_drain = nullptr;
		  _end = _at;
		return false;
	}
	 _at = window->begin();
	_end = window->end();
	return true;
}

auto ca::text_writer::_put_slow(const char* data, ca::usz size) noexcept -> void {
	for (;;) {
		const ca::usz part = ca::min(size, static_cast<ca::usz>(_end - _at));
		ca::copy_bytes(_at, data, part);
		 _at += part;
		data += part;
		size -= part;
		if (size == 0U || !this->_refill())
			return;
	}
}

auto ca::text_writer::_fill_slow(const char c, ca::usz count) noexcept -> void {
	for (;;) {
		const ca::usz part = ca::min(count, static_cast<ca::usz>(_end - _at));
		ca::fill_bytes(_at, static_cast<ca::u8>(c), part);
		  _at += part;
		count -= part;
		if (count == 0U || !this->_refill())
			return;
	}
}
