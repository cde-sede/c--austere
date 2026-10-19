/* The local time zone without libc: TZif files (RFC 8536, version 2 and later read through
   their 64-bit block) and POSIX TZ rules, chosen from TZ as glibc's tzset does. */

#include "1-primitives/atomic/atomic.hpp"
#include "1-primitives/atomic/memory_order.hpp"
#include "1-primitives/integers/byte.hpp"
#include "1-primitives/integers/u32.hpp"
#include "1-primitives/integers/u64.hpp"
#include "1-primitives/integers/u8.hpp"
#include "1-primitives/integers/usz.hpp"
#include "2-vocabulary/span.hpp"
#include "3-system/environment/getenv.hpp"
#include "3-system/file/close.hpp"
#include "3-system/file/open.hpp"
#include "3-system/file/read.hpp"
#include "3-system/file/stat.hpp"
#include "3-system/fs/options.hpp"
#include "3-system/memory/mmap_anonymous.hpp"
#include "3-system/memory/munmap.hpp"
#include "3-system/memory/protection.hpp"
#include "3-system/time/impl/civil.hpp"
#include "3-system/time/impl/time_zone.hpp"


// -- CA NAMESPACE -----------------------------------------------------------

namespace ca {


	// -- TIME ZONE ----------------------------------------------------------

	namespace sys {


		namespace impl {

			/* @DOC A day of a POSIX TZ rule: 'J' n (1 - 365, no February 29), 'N' n (0 - 365),
			   or 'M' month.week.day (week 5 the last); then the local time it applies at. */
			struct rule_date final {
					using self = ca::sys::impl::rule_date;
					char _kind;
					ca::i32 _number;
					ca::i32 _month;
					ca::i32 _week;
					ca::i32 _day;
					ca::i32 _time;
			}; // struct rule_date

			/* @DOC A POSIX TZ rule: offsets east of UTC, and when daylight saving starts and
			   ends. */
			struct posix_rule final {
					using self = ca::sys::impl::posix_rule;
					ca::i32 _standard;
					ca::i32 _daylight_offset;
					bool _has_daylight;
					bool _has_dates;
					ca::sys::impl::rule_date _start;
					ca::sys::impl::rule_date _end;
			}; // struct posix_rule

			/* @DOC A zone read once: TZif transitions (in a mapping of their own) and types
			   (offset, daylight saving, transition times in standard or universal time), then
			   the rule past them; or only a rule; or UTC. */
			struct loaded_zone final {
					using self = ca::sys::impl::loaded_zone;
					ca::usz _transitions;
					ca::i64* _times;
					ca::u8* _indices;
					ca::usz _types;
					ca::i32 _offsets[256];
					bool _daylight[256];
					bool _standard_time[256];
					bool _universal_time[256];
					bool _has_rule;
					ca::sys::impl::posix_rule _rule;
					ca::span<ca::byte> _mapping;
			}; // struct loaded_zone

			/* @DOC The file a zone was read from and its identity then. With TZ unset a change to
			   it (/etc/localtime replaced) reloads the zone; a TZ kept the same is not re-read,
			   as with glibc's tzset. */
			struct zone_file final {
					using self = ca::sys::impl::zone_file;
					char _path[288];
					ca::u64 _device;
					ca::u64 _inode;
					ca::u64 _size;
					ca::i64 _modified;
					bool _valid;
			}; // struct zone_file

		} // namespace impl

	} // namespace sys

} // namespace ca

static constexpr ca::usz KEY = 256U;
static constexpr ca::usz FILE_LIMIT = ca::usz{1} << 20U;
static constexpr ca::usz FILE_PATH = KEY + 32U;

static_assert(sizeof(ca::sys::impl::zone_file::_path) == FILE_PATH);

static ca::sys::impl::loaded_zone cached{}; // file-state: ignore
static ca::sys::impl::zone_file cached_file{}; // file-state: ignore
static char cached_key[KEY]; // file-state: ignore
static bool cached_valid = false; // file-state: ignore
static bool cached_unset = false; // file-state: ignore
static constinit ca::atomic<int> zone_lock{0}; // file-state: ignore


// -- POSIX RULES ------------------------------------------------------------

static auto is_digit(const char c) noexcept -> bool {
	return c >= '0' && c <= '9';
}

static auto parse_name(const char*& at) noexcept -> bool {

	if (*at == '<') {
		++at;
		while (*at != '\0' && *at != '>')
			++at;
		if (*at != '>')
			return false;
		++at;
		return true;
	}
	const char* const start = at;
	while ((*at >= 'a' && *at <= 'z') || (*at >= 'A' && *at <= 'Z'))
		++at;
	return at - start >= 3;
}

static auto parse_number(const char*& at, ca::i32& value) noexcept -> bool {

	if (!is_digit(*at))
		return false;
	value = 0;
	while (is_digit(*at) && value < 100000)
		value = value * 10 + (*at++ - '0');
	return true;
}

/* [+-]hh[:mm[:ss]] as seconds. */
static auto parse_time(const char*& at, ca::i32& seconds) noexcept -> bool {

	ca::i32 sign = 1;
	if (*at == '+' || *at == '-')
		sign = *at++ == '-' ? -1 : 1;
	ca::i32 hours;
	if (!parse_number(at, hours))
		return false;
	ca::i32 minutes = 0;
	ca::i32 rest = 0;
	if (*at == ':') {
		++at;
		if (!parse_number(at, minutes))
			return false;
		if (*at == ':') {
			++at;
			if (!parse_number(at, rest))
				return false;
		}
	}
	seconds = sign * (hours * 3600 + minutes * 60 + rest);
	return true;
}

static auto parse_date(const char*& at, ca::sys::impl::rule_date& date) noexcept -> bool {

	date._time = 7200;
	if (*at == 'J') {
		++at;
		date._kind = 'J';
		if (!parse_number(at, date._number) || date._number < 1 || date._number > 365)
			return false;
	} else if (*at == 'M') {
		++at;
		date._kind = 'M';
		if (!parse_number(at, date._month) || *at++ != '.' || !parse_number(at, date._week) || *at++ != '.' || !parse_number(at, date._day))
			return false;
		if (date._month < 1 || date._month > 12 || date._week < 1 || date._week > 5 || date._day > 6)
			return false;
	} else {
		date._kind = 'N';
		if (!parse_number(at, date._number) || date._number > 365)
			return false;
	}
	if (*at == '/') {
		++at;
		if (!parse_time(at, date._time))
			return false;
	}
	return true;
}

static auto parse_rule(const char* text, ca::sys::impl::posix_rule& rule) noexcept -> bool {

	const char* at = text;
	ca::i32 offset;
	if (!parse_name(at) || !parse_time(at, offset))
		return false;
	    rule._standard = -offset;
	rule._has_daylight = false;
	if (*at == '\0')
		return true;
	if (!parse_name(at))
		return false;
	   rule._has_daylight = true;
	rule._daylight_offset = rule._standard + 3600;
	if (*at != '\0' && *at != ',') {
		if (!parse_time(at, offset))
			return false;
		rule._daylight_offset = -offset;
	}
	rule._has_dates = *at == ',';
	if (*at == '\0') {
		const char* fallback = "M3.2.0,M11.1.0";
		return parse_date(fallback, rule._start) && *fallback++ == ',' && parse_date(fallback, rule._end);
	}
	if (*at++ != ',' || !parse_date(at, rule._start) || *at++ != ',' || !parse_date(at, rule._end))
		return false;
	return *at == '\0';
}

static auto is_leap(const ca::i64 year) noexcept -> bool {
	return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

static auto month_length(const ca::i32 month, const bool leap) noexcept -> ca::i32 {

	constexpr ca::i32 lengths[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	return lengths[month - 1] + (month == 2 && leap ? 1 : 0);
}

/* The UTC instant date falls on in year, its time read in the zone at utc_offset. As glibc's
   compute_change: the day within the year from year's own calendar, counted from January 1st of
   year, or of 1970 for earlier years (so a rule gives no daylight saving before 1970 in the
   north, and always in the south). */
static auto instant_of(const ca::sys::impl::rule_date& date, const ca::i64 year, const ca::i32 utc_offset) noexcept -> ca::i64 {

	const bool leap = is_leap(year);
	const ca::i64 january = ca::sys::impl::days_from_civil(year, 1U, 1U);
	ca::i64 day_of_year;
	if (date._kind == 'J') {
		day_of_year = date._number - 1 + (leap && date._number >= 60 ? 1 : 0);
	} else if (date._kind == 'N') {
		day_of_year = date._number;
	} else {
		const ca::i64 first = ca::sys::impl::days_from_civil(year, static_cast<ca::u32>(date._month), 1U);
		const ca::i64 weekday = ((first + 4) % 7 + 7) % 7;
		ca::i64 offset = (date._day - weekday + 7) % 7 + (date._week - 1) * 7;
		while (offset >= month_length(date._month, leap))
			offset -= 7;
		day_of_year = first + offset - january;
	}
	const ca::i64 base = year > 1970 ? january : 0;
	return (base + day_of_year) * 86400 + date._time - utc_offset;
}

static auto rule_offset(const ca::sys::impl::posix_rule& rule, const ca::i64 seconds) noexcept -> ca::sys::impl::zone_offset {

	if (!rule._has_daylight)
		return {rule._standard, false};
	const ca::i64 days = (seconds >= 0 ? seconds : seconds - 86399) / 86400;
	const ca::i64 year = ca::sys::impl::civil_from_days(days)._year;
	const ca::i64 start = instant_of(rule._start, year, rule._standard);
	const ca::i64 end = instant_of(rule._end, year, rule._daylight_offset);
	const bool daylight = start < end ? (seconds >= start && seconds < end) : (seconds < end || seconds >= start);
	return daylight ? ca::sys::impl::zone_offset{rule._daylight_offset, true} : ca::sys::impl::zone_offset{rule._standard, false};
}


// -- TZIF FILES -------------------------------------------------------------

static auto big32(const ca::u8* const at) noexcept -> ca::u32 {
	return static_cast<ca::u32>(at[0]) << 24U | static_cast<ca::u32>(at[1]) << 16U | static_cast<ca::u32>(at[2]) << 8U | at[3];
}

static auto big64(const ca::u8* const at) noexcept -> ca::u64 {
	return static_cast<ca::u64>(big32(at)) << 32U | big32(at + 4);
}

/* The whole file at path into buffer; its size, 0 when it cannot be read. */
static auto read_file(const char* const path, ca::span<ca::byte> buffer) noexcept -> ca::usz {

	const auto fd = ca::sys::open(path, ca::fs::cloexec_t::OPEN_VALUE, 0U);
	if (!fd)
		return 0U;
	ca::usz filled = 0U;
	for (;;) {
		const auto done = ca::sys::read(*fd, buffer.sub(filled, buffer.size() - filled));
		if (!done || *done == 0U || filled + *done == buffer.size()) {
			if (done)
				filled += *done;
			break;
		}
		filled += *done;
	}
	static_cast<void>(ca::sys::close(*fd));
	return filled;
}

/* zone from the TZif data, version 2 or later through its 64-bit block; false when malformed. */
static auto parse_tzif(const ca::u8* const data, const ca::usz size, ca::sys::impl::loaded_zone& zone) noexcept -> bool {

	if (size < 44U || data[0] != 'T' || data[1] != 'Z' || data[2] != 'i' || data[3] != 'f')
		return false;
	const ca::u8* header = data;
	ca::usz time_size = 4U;
	if (data[4] >= '2') {
		const ca::usz skip = 44U + big32(data + 32) * 5U + big32(data + 36) * 6U + big32(data + 40) + big32(data + 28) * 8U + big32(data + 24) + big32(data + 20);
		if (skip + 44U > size)
			return false;
		   header = data + skip;
		time_size = 8U;
	}
	const ca::usz utc_count = big32(header + 20);
	const ca::usz standard_count = big32(header + 24);
	const ca::usz leap_count = big32(header + 28);
	const ca::usz transitions = big32(header + 32);
	const ca::usz types = big32(header + 36);
	const ca::usz characters = big32(header + 40);
	const ca::u8* const times = header + 44;
	const ca::u8* const indices = times + transitions * time_size;
	const ca::u8* const records = indices + transitions;
	const ca::u8* const end = records + types * 6U + characters + leap_count * (time_size + 4U) + standard_count + utc_count;
	if (types == 0U || types > 256U || end > data + size)
		return false;
	const ca::usz bytes = transitions * (sizeof(ca::i64) + 1U);
	if (bytes != 0U) {
		const auto mapping = ca::sys::mmap_anonymous(bytes, ca::sys::protection::read | ca::sys::protection::write);
		if (!mapping)
			return false;
		zone._mapping = *mapping;
		ca::i64* const out_times = reinterpret_cast<ca::i64*>(mapping->data());
		ca::u8* const out_indices = reinterpret_cast<ca::u8*>(out_times + transitions);
		for (ca::usz i = 0U; i < transitions; ++i) {
			  out_times[i] = time_size == 8U ? static_cast<ca::i64>(big64(times + i * 8U)) : static_cast<ca::i64>(static_cast<ca::i32>(big32(times + i * 4U)));
			out_indices[i] = indices[i] < types ? indices[i] : 0U;
		}
		  zone._times = out_times;
		zone._indices = out_indices;
	}
	zone._transitions = transitions;
	      zone._types = types;
	const ca::u8* const standard_flags = records + types * 6U + characters + leap_count * (time_size + 4U);
	const ca::u8* const universal_flags = standard_flags + standard_count;
	for (ca::usz i = 0U; i < types; ++i) {
		       zone._offsets[i] = static_cast<ca::i32>(big32(records + i * 6U));
		      zone._daylight[i] = records[i * 6U + 4U] != 0U;
		 zone._standard_time[i] = i < standard_count && standard_flags[i] != 0U;
		zone._universal_time[i] = i < utc_count && universal_flags[i] != 0U;
	}
	zone._has_rule = false;
	if (time_size == 8U && end < data + size && *end == '\n') {
		char footer[KEY];
		ca::usz length = 0U;
		for (const ca::u8* at = end + 1; at < data + size && *at != '\n' && length + 1U < KEY; ++at)
			footer[length++] = static_cast<char>(*at);
		footer[length] = '\0';
		zone._has_rule = length != 0U && parse_rule(footer, zone._rule);
	}
	return true;
}

static auto release(ca::sys::impl::loaded_zone& zone) noexcept -> void {

	if (zone._mapping.size() != 0U)
		static_cast<void>(ca::sys::munmap(zone._mapping));
	zone = ca::sys::impl::loaded_zone{};
}

/* Whether path still names the file it did when identity was taken: same device, inode, size
   and modification time. */
static auto unchanged(const ca::sys::impl::zone_file& file) noexcept -> bool {

	const auto status = ca::sys::stat(file._path);
	return status && status->device() == file._device && status->inode() == file._inode && status->size() == file._size
		&& status->modified() == file._modified;
}

/* zone from the TZif file name (absolute, or under /usr/share/zoneinfo), its path and identity
   into file when not null; false, zone released, when it cannot be read. */
static auto read_zone(const char* const name, ca::sys::impl::loaded_zone& zone, ca::sys::impl::zone_file* const file) noexcept -> bool {

	char path[FILE_PATH];
	ca::usz length = 0U;
	if (*name != '/') {
		for (const char* at = "/usr/share/zoneinfo/"; *at != '\0'; ++at)
			path[length++] = *at;
	}
	for (const char* at = name; *at != '\0' && length + 1U < sizeof(path); ++at)
		path[length++] = *at;
	path[length] = '\0';
	const auto status = ca::sys::stat(path);
	const auto buffer = ca::sys::mmap_anonymous(FILE_LIMIT, ca::sys::protection::read | ca::sys::protection::write);
	if (!status || !buffer)
		return false;
	const ca::usz size = read_file(path, *buffer);
	const bool read = size != 0U && parse_tzif(reinterpret_cast<const ca::u8*>(buffer->data()), size, zone);
	static_cast<void>(ca::sys::munmap(*buffer));
	if (!read) {
		release(zone);
		return false;
	}
	if (file != nullptr) {
		for (ca::usz i = 0U; i <= length; ++i)
			file->_path[i] = path[i];
		  file->_device = status->device();
		   file->_inode = status->inode();
		    file->_size = status->size();
		file->_modified = status->modified();
		   file->_valid = true;
	}
	return true;
}

/* A rule naming daylight saving without dates takes the transitions of posixrules with the
   rule's own offsets, as glibc's __tzfile_default: two types left, standard and daylight. A
   transition in local wall time made in standard time moves by the rule's standard offset
   less the file's (its last standard one); one made in daylight time by the rule's daylight
   offset, the file's taken as 0: what glibc 2.39 does, measured transition by transition
   (spring and fall, several zones). Past the last transition the file's own footer applies,
   its offsets included, as in glibc. */
static auto default_transitions(ca::sys::impl::loaded_zone& zone) noexcept -> void {

	ca::sys::impl::loaded_zone rules{};
	if (!read_zone("posixrules", rules, nullptr))
		return;
	if (rules._types < 2U) {
		release(rules);
		return;
	}
	ca::i32 rule_standard = 0;
	for (ca::usz i = rules._transitions; i-- > 0U;) {
		const ca::u8 type = rules._indices[i];
		if (!rules._daylight[type]) {
			rule_standard = rules._offsets[type];
			break;
		}
	}
	bool daylight = false;
	for (ca::usz i = 0U; i < rules._transitions; ++i) {
		const ca::u8 type = rules._indices[i];
		if (rules._universal_time[type]) {
		} else if (daylight && !rules._standard_time[type]) {
			rules._times[i] += zone._rule._daylight_offset;
		} else {
			rules._times[i] += zone._rule._standard - rule_standard;
		}
		         daylight = rules._daylight[type];
		rules._indices[i] = daylight ? 1U : 0U;
	}
	      rules._types = 2U;
	 rules._offsets[0] = zone._rule._standard;
	 rules._offsets[1] = zone._rule._daylight_offset;
	rules._daylight[0] = false;
	rules._daylight[1] = true;
	              zone = rules;
}

static auto load(const char* const tz, ca::sys::impl::loaded_zone& zone, ca::sys::impl::zone_file& file) noexcept -> void {

	release(zone);
	file._valid = false;
	const char* name = tz == nullptr ? "/etc/localtime" : tz;
	if (*name == ':')
		++name;
	if (*name == '\0' || read_zone(name, zone, &file) || tz == nullptr)
		return;
	zone._has_rule = parse_rule(name, zone._rule);
	if (zone._has_rule && zone._rule._has_daylight && !zone._rule._has_dates)
		default_transitions(zone);
}

static auto offset_in(const ca::sys::impl::loaded_zone& zone, const ca::i64 seconds) noexcept -> ca::sys::impl::zone_offset {

	if (zone._types == 0U)
		return zone._has_rule ? rule_offset(zone._rule, seconds) : ca::sys::impl::zone_offset{0, false};
	if (zone._transitions == 0U || seconds < zone._times[0]) {
		if (zone._transitions == 0U && zone._has_rule)
			return rule_offset(zone._rule, seconds);
		ca::usz type = 0U;
		while (type < zone._types && zone._daylight[type])
			++type;
		if (type == zone._types)
			type = 0U;
		return {zone._offsets[type], zone._daylight[type]};
	}
	if (seconds >= zone._times[zone._transitions - 1U] && zone._has_rule)
		return rule_offset(zone._rule, seconds);
	ca::usz low = 0U;
	ca::usz high = zone._transitions;
	while (high - low > 1U) {
		const ca::usz middle = low + (high - low) / 2U;
		if (zone._times[middle] <= seconds)
			low = middle;
		else
			high = middle;
	}
	const ca::u8 type = zone._indices[low];
	return {zone._offsets[type], zone._daylight[type]};
}

[[nodiscard]] auto ca::sys::impl::zone_offset_at(const ca::i64 seconds) noexcept -> ca::sys::impl::zone_offset {

	const auto tz = ca::sys::getenv("TZ");
	const char* const text = tz ? tz->data() : nullptr;
	while (zone_lock.exchange<ca::memory_order::acquire>(1) != 0) {
	}
	bool same = cached_valid && cached_unset == (text == nullptr);
	if (same && text != nullptr) {
		ca::usz i = 0U;
		for (; i + 1U < KEY && text[i] != '\0' && text[i] == cached_key[i]; ++i) {
		}
		same = text[i] == cached_key[i];
	}
	if (same && cached_unset && cached_file._valid)
		same = unchanged(cached_file);
	if (!same) {
		load(text, cached, cached_file);
		cached_unset = text == nullptr;
		ca::usz length = 0U;
		for (; text != nullptr && text[length] != '\0' && length + 1U < KEY; ++length)
			cached_key[length] = text[length];
		cached_key[length] = '\0';
		      cached_valid = text == nullptr || text[length] == '\0';
	}
	const ca::sys::impl::zone_offset offset = offset_in(cached, seconds);
	zone_lock.store<ca::memory_order::release>(0);
	return offset;
}
