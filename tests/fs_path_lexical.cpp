// expect: exit 0
#include "5-collections/fs/path.hpp"

static auto is(const ca::sv::str a, const char* b) noexcept -> bool {
	return a == ca::sv::str{b};
}

auto main(void) -> int {
	auto p = ca::fs::path::from("//usr/./lib//archive.tar.gz/");
	if (!p || !is(p->view(), "/usr/lib/archive.tar.gz") || !p->is_absolute())
		return 1;
	if (!is(p->name(), "archive.tar.gz") || !is(p->suffix(), ".gz") || !is(p->stem(), "archive.tar") || !is(p->parent(), "/usr/lib"))
		return 2;
	const char* expected[] = {"/", "usr", "lib", "archive.tar.gz"};
	ca::usz i = 0U;
	for (const ca::sv::str part : p->components()) {
		if (i == 4U || !is(part, expected[i++]))
			return 3;
	}
	if (i != 4U)
		return 4;
	const ca::fs::path dot;
	if (!is(dot.view(), ".") || !dot.name().empty() || !is(dot.parent(), ".") || dot.components().begin() != dot.components().end())
		return 5;
	auto rel = ca::fs::path::from("a/b");
	if (!rel || !is(rel->parent(), "a") || !is(rel->root(), ""))
		return 6;
	auto joined = rel->join("../c/");
	auto replaced = rel->join("/etc");
	if (!joined || !is(joined->view(), "a/b/../c") || !replaced || !is(replaced->view(), "/etc"))
		return 7;
	auto zip = p->with_suffix(".zip");
	auto named = p->with_name("x");
	auto stemmed = p->with_stem("y");
	if (!zip || !is(zip->view(), "/usr/lib/archive.tar.zip") || !named || !is(named->view(), "/usr/lib/x")
		|| !stemmed || !is(stemmed->view(), "/usr/lib/y.gz"))
		return 8;
	auto base = ca::fs::path::from("/usr");
	if (!base)
		return 9;
	auto below = p->relative_to(*base);
	if (!below || !is(*below, "lib/archive.tar.gz") || rel->is_relative_to(*base) || !p->is_relative_to(*p))
		return 10;
	if (!p->match("*.gz") || !p->match("lib/*.tar.*") || !p->match("/usr/*/a[q-s]chive.*") || p->match("/lib/*") || p->match("[!a]*"))
		return 11;
	auto hidden = ca::fs::path::from(".bashrc");
	if (!hidden || !hidden->suffix().empty() || !is(hidden->stem(), ".bashrc"))
		return 12;
	auto copy = p->clone();
	if (!copy || !(*copy == *p) || !p->push("more") || !is(p->view(), "/usr/lib/archive.tar.gz/more"))
		return 13;
	auto root = ca::fs::path::from("/");
	if (!root || !is(root->parent(), "/") || !root->name().empty())
		return 14;
	return 0;
}
