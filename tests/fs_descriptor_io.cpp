// expect: exit 0
#include "6-resources/fs/descriptor.hpp"

auto main(void) -> int {
	constexpr auto rw = ca::fs::readable | ca::fs::writable | ca::fs::create | ca::fs::truncate | ca::fs::cloexec;
	auto d = ca::fs::descriptor::open("build/fs_descriptor_io.tmp", rw, 0600U);
	if (!d)
		return 1;
	if (!d->write_all(ca::sv::str{"hello, world"}))
		return 2;
	auto twin = d->duplicate(ca::fs::cloexec);
	if (!twin)
		return 3;
	auto at = twin->seek(7);
	if (!at || *at != 7)
		return 4;
	auto rest = d->read_all();
	if (!rest || rest->size() != 5U || (*rest)[0] != 'w')
		return 5;
	auto status = d->stat();
	if (!status || status->size() != 12U || status->kind() != ca::sys::file_kind::regular)
		return 6;
	if (!d->sync() || !twin->destroy() || !d->destroy() || d->is_open())
		return 7;
	auto gone = ca::fs::path::from("build/fs_descriptor_io.tmp").and_then([](const ca::fs::path& p) {
		return p.unlink();
	});
	return gone ? 0 : 8;
}
