#include "1-primitives/config/architecture.hpp"
#include "1-primitives/config/platform.hpp"
#include "1-primitives/debug/frame_address.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/io/io_buffer.hpp"
#include "3-system/syscall/auxiliary.hpp"
#include "3-system/syscall/syscalls.hpp"
#include "3-system/trace/backtrace.hpp"


#if ca_platform(linux) && ca_architecture(x86_64)

namespace {

	struct program_header final {

			using self = program_header;

			ca::u32 _type;
			ca::u32 _flags;
			ca::u64 _offset;
			ca::u64 _address;
			ca::u64 _physical;
			ca::u64 _file_size;
			ca::u64 _memory_size;
			ca::u64 _alignment;

	}; // struct program_header

	constexpr ca::u64 AT_PHDR = 3U;
	constexpr ca::u64 AT_PHNUM = 5U;
	constexpr ca::u32 PT_LOAD = 1U;
	constexpr ca::u32 PT_PHDR = 6U;

	constexpr ca::usz ELF_HEADER_SIZE = 64U;
	constexpr ca::usz ELF_PHOFF = 32U;

	/* Copies size bytes at address into out through the kernel, which fails instead of faulting
	   on unmapped memory; raw, so ca's last error stays. */
	auto read_own(const ca::usz address, void* const out, const ca::usz size) noexcept -> bool {

		const ca::sys::io_buffer local{ca::span<const ca::byte>{static_cast<const ca::byte*>(out), size}};
		const ca::sys::io_buffer remote{ca::span<const ca::byte>{reinterpret_cast<const ca::byte*>(address), size}};
		const ca::isz pid = ca::syscall::impl::enter(ca::syscall::number::getpid);
		const ca::isz read = ca::syscall::impl::enter(ca::syscall::number::process_vm_readv, pid, reinterpret_cast<ca::isz>(&local), 1,
		                                              reinterpret_cast<ca::isz>(&remote), 1, 0);
		return read == static_cast<ca::isz>(size);
	}

	/* Load bias of an executable without PT_PHDR (static-pie): its ELF header sits right before
	   its program headers, in the segment mapping file offset 0. */
	auto bias_from_header(const ca::u64 table, const program_header* const headers, const ca::u64 count, ca::usz& bias) noexcept -> bool {

		unsigned char header[ELF_PHOFF + sizeof(ca::u64)];
		if (table < ELF_HEADER_SIZE || !read_own(table - ELF_HEADER_SIZE, header, sizeof(header)))
			return false;
		ca::u64 phoff = 0U;
		for (ca::usz i = 0U; i < sizeof(ca::u64); ++i)
			phoff |= static_cast<ca::u64>(header[ELF_PHOFF + i]) << (8U * i);
		if (header[0] != 0x7FU || header[1] != 'E' || header[2] != 'L' || header[3] != 'F' || phoff != ELF_HEADER_SIZE)
			return false;
		for (ca::u64 i = 0U; i < count; ++i) {
			if (headers[i]._type == PT_LOAD && headers[i]._offset == 0U) {
				bias = table - ELF_HEADER_SIZE - headers[i]._address;
				return true;
			}
		}
		return false;
	}

} // namespace

#endif


auto ca::trace::backtrace::capture(void) noexcept -> ca::trace::backtrace {
	return self::walk(ca::impl::frame_address());
}

auto ca::trace::backtrace::walk(const void* const frame) noexcept -> ca::trace::backtrace {

	self out{};
#if ca_platform(linux) && ca_architecture(x86_64)
	ca::usz at = reinterpret_cast<ca::usz>(frame);
	while (out._size < self::CAPACITY && at != 0U && at % sizeof(ca::usz) == 0U) {
		ca::usz record[2];
		if (!read_own(at, record, sizeof(record)) || record[1] == 0U)
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
#if ca_platform(linux) && ca_architecture(x86_64)
	const ca::u64 table = ca::syscall::auxiliary(AT_PHDR);
	const ca::u64 count = ca::syscall::auxiliary(AT_PHNUM);
	if (table == 0U || count == 0U)
		return range;
	const program_header* const headers = reinterpret_cast<const program_header*>(table);
	bool found = false;
	for (ca::u64 i = 0U; i < count; ++i) {
		if (headers[i]._type == PT_PHDR) {
			range._bias = table - headers[i]._address;
			      found = true;
		}
	}
	if (!found && !bias_from_header(table, headers, count, range._bias))
		return range;
	range._begin = ~ca::usz{0U};
	for (ca::u64 i = 0U; i < count; ++i) {
		if (headers[i]._type != PT_LOAD)
			continue;
		const ca::usz begin = range._bias + headers[i]._address;
		const ca::usz end   = begin + headers[i]._memory_size;
		if (begin < range._begin)
			range._begin = begin;
		if (end > range._end)
			range._end = end;
	}
	if (range._end == 0U)
		range._begin = 0U;
#endif
	return range;
}
