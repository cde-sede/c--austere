#include "1-primitives/integers/byte.hpp"
#include "1-primitives/memory/copy_bytes.hpp"
#include "1-primitives/memory/find_byte.hpp"
#include "1-primitives/memory/relocate_overlapping_n.hpp"
#include "2-vocabulary/failure.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/error/errc.hpp"
#include "3-system/file/read.hpp"
#include "6-resources/io/reader.hpp"


[[nodiscard]] auto ca::io::reader::_refill(void) noexcept -> ca::result<ca::usz> {

	if (_begin == _end) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	if (_tie != nullptr)
		static_cast<void>(_tie->write_pending());
	    _at = _begin;
	_filled = _begin;
	auto got = ca::sys::read(_fd, this->buffer());
	if (got)
		_filled += *got;
	return got;
}

[[nodiscard]] auto ca::io::reader::_read_empty(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

	if (buffer.size() >= static_cast<ca::usz>(_end - _begin)) {
		if (_tie != nullptr)
			static_cast<void>(_tie->write_pending());
		return ca::sys::read(_fd, buffer);
	}
	auto got = this->_refill();
	if (!got || *got == 0U)
		return got;
	const ca::usz count = buffer.size() < *got ? buffer.size() : *got;
	ca::copy_bytes(buffer.data(), _at, count);
	_at += count;
	return count;
}

[[nodiscard]] auto ca::io::reader::read_full(const ca::span<ca::byte> buffer) noexcept -> ca::result<ca::usz> {

	ca::usz done = 0U;
	while (done < buffer.size()) {
		auto got = this->read(buffer.without_prefix(done));
		if (!got) [[unlikely]]
			return got;
		if (*got == 0U)
			break;
		done += *got;
	}
	return done;
}

[[nodiscard]] auto ca::io::reader::read_until(const char delimiter) noexcept -> ca::result<ca::sv::str> {

	if (_begin == _end) [[unlikely]]
		return ca::failure{ca::errc::invalid_argument};
	ca::usz scanned = 0U;
	for (;;) {
		const char* const found = ca::find_byte(_at + scanned, delimiter, static_cast<ca::usz>(_filled - _at) - scanned);
		if (found != nullptr) {
			const ca::sv::str part{_at, static_cast<ca::usz>(found + 1 - _at)};
			_at += part.size();
			return part;
		}
		scanned = static_cast<ca::usz>(_filled - _at);
		if (_at != _begin && _filled == _end) {
			ca::relocate_overlapping_n(_at, scanned, _begin);
			     _at = _begin;
			 _filled = _begin + scanned;
		}
		else if (_at == _filled) {
			     _at = _begin;
			 _filled = _begin;
		}
		if (_filled == _end) {
			const ca::sv::str part{_at, scanned};
			_at = _filled;
			return part;
		}
		if (_tie != nullptr)
			static_cast<void>(_tie->write_pending());
		auto got = ca::sys::read(_fd, ca::span{_filled, static_cast<ca::usz>(_end - _filled)});
		if (!got) [[unlikely]]
			return got.propagate();
		if (*got == 0U) {
			const ca::sv::str part{_at, scanned};
			_at = _filled;
			return part;
		}
		_filled += *got;
	}
}

[[nodiscard]] auto ca::io::reader::set_buffer(const ca::span<char> buffer) noexcept -> ca::result<void> {

	const ca::usz unread = static_cast<ca::usz>(_filled - _at);
	if (unread > buffer.size()) [[unlikely]]
		return ca::failure{ca::errc::no_space};
	ca::relocate_overlapping_n(_at, unread, buffer.data());
	 _begin = buffer.begin();
	    _at = buffer.begin();
	_filled = buffer.begin() + unread;
	   _end = buffer.end();
	return ca::result<void>{};
}
