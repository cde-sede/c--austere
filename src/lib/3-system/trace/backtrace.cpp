#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/debug/frame_address.hpp"
#include "1-primitives/integers/byte.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/elf/image.hpp"
#include "3-system/elf/program_header.hpp"
#include "3-system/syscall/impl/read_own.hpp"
#include "3-system/trace/backtrace.hpp"


auto ca::trace::backtrace::capture(void) noexcept -> ca::trace::backtrace {
	return self::walk(ca::impl::frame_address());
}

auto ca::trace::backtrace::walk(const void* const frame) noexcept -> ca::trace::backtrace {

	self out{};
#if ca_platform(linux) && ca_architecture(x86_64)
	ca::usz at = reinterpret_cast<ca::usz>(frame);
	while (out._size < self::CAPACITY && at != 0U && at % sizeof(ca::usz) == 0U) {
		ca::usz record[2];
		if (!ca::syscall::impl::read_own(at, ca::span<ca::byte>{reinterpret_cast<ca::byte*>(record), sizeof(record)}) || record[1] == 0U)
			break;
		out._addresses[out._size++] = record[1] - 1U;
		if (record[0] <= at)
			break;
		at = record[0];
	}
#else
	static_cast<void>(frame);
#endif
	return out;
}

auto ca::trace::impl::executable(void) noexcept -> ca::trace::impl::executable_range {

	ca::trace::impl::executable_range range{};
	const auto program = ca::elf::image::program();
	if (!program)
		return range;
	 range._bias = program->bias();
	range._begin = ~ca::usz{0U};
	for (const ca::elf::program_header& segment : program->segments()) {
		if (segment._type != ca::elf::segment_types::load)
			continue;
		const ca::usz begin = range._bias + segment._address;
		const ca::usz end   = begin + segment._memory_size;
		if (begin < range._begin)
			range._begin = begin;
		if (end > range._end)
			range._end = end;
	}
	if (range._end == 0U)
		range._begin = 0U;
	return range;
}
