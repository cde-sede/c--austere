// expect: exit 0
#include "3-system/error/errc.hpp"
#include "5-collections/fs/path.hpp"

static auto count(const ca::fs::path& root, const char* pattern, const bool recursive) noexcept -> ca::usz {

	auto found = recursive ? root.rglob(pattern) : root.glob(pattern);
	if (!found)
		return 999U;
	const ca::usz size = found->size();
	static_cast<void>(found->cleanup());
	return size;
}

auto main(void) -> int {
	auto root = ca::fs::path::from("build/fs_path_files.tmp");
	if (!root)
		return 1;
	auto deep = root->join("a/b/c");
	if (!deep || !deep->mkdirs() || !deep->mkdirs())
		return 2;
	auto again = deep->mkdir();
	if (again || again.error() != ca::errc::exists)
		return 3;
	auto file = deep->join("notes.txt");
	auto other = root->join("a/top.txt");
	if (!file || !other || !file->write_text("line\n") || !other->touch())
		return 4;
	auto text = file->read_text();
	if (!text || text->size() != 5U || (*text)[0] != 'l')
		return 5;
	auto is_file = file->is_file();
	auto is_dir = deep->is_directory();
	auto missing = root->join("nope");
	if (!is_file || !*is_file || !is_dir || !*is_dir || !missing)
		return 6;
	auto exists = missing->exists();
	if (!exists || *exists)
		return 7;
	auto a = root->join("a");
	if (!a)
		return 8;
	auto entries = a->iterdir();
	if (!entries || entries->size() != 2U || !entries->cleanup())
		return 9;
	if (count(*root, "a/*.txt", false) != 1U || count(*root, "**/*.txt", false) != 2U || count(*root, "*.txt", true) != 2U
		|| count(*root, "a/b/c", false) != 1U || count(*root, "*/[ab]", false) != 1U || count(*root, "x/*", false) != 0U)
		return 10;
	auto link = root->join("link");
	auto real = file->resolve();
	if (!link || !real || !real->is_absolute() || !link->symlink_to(*real))
		return 11;
	auto is_link = link->is_symlink();
	auto target = link->readlink();
	auto resolved = link->resolve();
	if (!is_link || !*is_link || !target || !(*target == *real) || !resolved || !(*resolved == *real))
		return 12;
	auto moved = deep->join("moved.txt");
	if (!moved || !file->rename(*moved) || !moved->chmod(0600U))
		return 13;
	auto status = moved->stat();
	if (!status || status->permissions() != 0600U)
		return 14;
	if (!link->unlink() || !moved->unlink() || !other->unlink() || !deep->rmdir())
		return 15;
	auto b = root->join("a/b");
	if (!b || !b->rmdir() || !a->rmdir() || !root->rmdir())
		return 16;
	auto cwd = ca::fs::path::cwd();
	auto absolute = root->absolute();
	if (!cwd || !absolute || !absolute->is_relative_to(*cwd))
		return 17;
	return 0;
}
