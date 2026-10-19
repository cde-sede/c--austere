// expect: exit 0
// link: both
#include "6-resources/fs/descriptor.hpp"

static auto drop(const char* const name) noexcept -> void {

	auto path = ca::fs::path::from(name);
	if (path) {
		static_cast<void>(path->unlink());
		static_cast<void>(path->cleanup());
	}
}

auto main(void) -> int {
	drop("build/fs_descriptor_positional.tmp");
	drop("build/fs_descriptor_positional.link");
	constexpr auto rw = ca::fs::readable | ca::fs::writable | ca::fs::create | ca::fs::truncate | ca::fs::cloexec;
	auto d = ca::fs::descriptor::open("build/fs_descriptor_positional.tmp", rw, 0600U);
	if (!d || !d->write_all(ca::sv::str{"0123456789"}))
		return 1;
	auto placed = d->write_at(ca::sv::str{"ab"}, 4U);
	char got[4]{};
	auto read = d->read_at(got, 3U);
	if (!placed || *placed != 2U || !read || *read != 4U || got[0] != '3' || got[1] != 'a' || got[3] != '6')
		return 2;
	auto offset = d->seek(0, ca::sys::seek_origin::current);
	if (!offset || *offset != 10)
		return 3;
	if (!d->resize(4U) || !d->allocate(0U, 8192U) || !d->sync_data() || !d->chmod(0640U))
		return 4;
	auto status = d->stat();
	if (!status || status->size() != 8192U || status->permissions() != 0640U)
		return 5;
	if (!d->chown(ca::sys::unchanged_id, ca::sys::unchanged_id))
		return 6;

	auto file = ca::fs::path::from("build/fs_descriptor_positional.tmp");
	auto link = ca::fs::path::from("build/fs_descriptor_positional.link");
	if (!file || !link || !link->hardlink_to(*file))
		return 7;
	auto linked = link->stat();
	if (!linked || linked->links() != 2U || linked->inode() != status->inode())
		return 8;
	auto again = link->hardlink_to(*file);
	if (again || again.error() != ca::errc::exists)
		return 9;
	if (!d->cleanup() || !link->unlink() || !file->unlink() || !link->cleanup() || !file->cleanup())
		return 10;
	return 0;
}
