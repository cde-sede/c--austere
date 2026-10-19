#!/bin/sh
# Runs one test, tests/NAME.cpp; called by `make test` once per test (in
# parallel under -j), which sets CXX, MODE, CXXFLAGS, LDFLAGS, LIB, BUILD and,
# in `make matrix`, TAG (the configuration, put before each line). Prints one
# line and writes BUILD/tests/NAME.result (pass, fail or skip), which
# tooling/test_summary.sh counts. Exits 0 unless it cannot run at all.
#
#   tooling/test.sh tests/NAME.cpp
#
# The first lines of each test say what must happen:
#
#   // expect: exit N          builds, runs, exits with status N
#   // expect: error REGEX     does not build; diagnostics match REGEX (grep -E)
#   // expect: trap            builds, then dies by a trap (SIGILL or SIGTRAP)
#   // mode: debug             only checked in debug builds (or: release)
#   // link: freestanding      linked without libc, ca's own _start and runtime (RUNTIME,
#                              built from src/runtime/), no startup files: twice, -static
#                              and -static-pie, both run
#   // link: libc              against libc only (threads, or libc compared on purpose)
#   // link: both              checked twice: against libc, then freestanding; what a test
#                              that runs (exit, trap) does without a link line

ulimit -c 0
test=$1
name=$(basename "$test" .cpp)
bin=$BUILD/tests/$name
result=$BUILD/tests/$name.result
tag=${TAG:+"[$TAG] "}
mkdir -p "$BUILD/tests"

expect=$(sed -n 's|^// expect: ||p' "$test")
kind=${expect%% *}
arg=${expect#"$kind"}
arg=${arg# }
mode=$(sed -n 's|^// mode: ||p' "$test")

if [ -n "$mode" ] && [ "$mode" != "$MODE" ]; then
	printf '  skip  %s%-36s %s builds only\n' "$tag" "$name" "$mode"
	echo skip > "$result"
	exit 0
fi

# check MODE BIN: builds the test as BIN, against libc (MODE libc) or without it (freestanding),
# runs it, and sets failure (empty when it behaved as expected), out and built.
check() {
	if [ "$1" = freestanding ]; then
		out=$($CXX $CXXFLAGS -c "$test" -o "$2.o" 2>&1) &&
		out=$($CXX "$2.o" -o "$2" -nostdlib -static -Wl,-u,_start \
			-Wl,--start-group "$LIB" "$RUNTIME" -Wl,--end-group 2>&1) &&
		out=$($CXX "$2.o" -o "$2_pie" -nostdlib -static-pie -Wl,-u,_start \
			-Wl,--start-group "$LIB" "$RUNTIME" -Wl,--end-group 2>&1)
	else
		out=$($CXX $CXXFLAGS "$test" -o "$2" "$LIB" $LDFLAGS 2>&1)
	fi
	built=$?
	failure=
	case $kind in
	error)
		if [ $built -eq 0 ]; then
			failure="built, but must not"
		elif ! printf '%s\n' "$out" | grep -Eq -- "$arg"; then
			failure="rejected, but not with /$arg/"
		fi ;;
	exit|trap)
		if [ $built -ne 0 ]; then
			failure="does not build"
		else
			status=$("$2" >/dev/null 2>&1; echo $?)
			if [ "$1" = freestanding ] && [ "$status" = "$arg" ]; then
				pie=$("$2_pie" >/dev/null 2>&1; echo $?)
				[ "$pie" != "$status" ] && status="$pie (static-pie)"
			fi
			if [ "$kind" = exit ] && [ "$status" != "$arg" ]; then
				failure="expected exit $arg, got status $status"
			elif [ "$kind" = trap ] && [ "$status" != 132 ] && [ "$status" != 133 ]; then
				failure="expected a trap, got status $status"
			fi
		fi ;;
	*)
		failure='no "// expect:" line' ;;
	esac
	[ "$1" = freestanding ] && [ -n "$failure" ] && failure="(freestanding) $failure"
}

link=$(sed -n 's|^// link: ||p' "$test")
[ -z "$link" ] && [ "$kind" != error ] && link=both
case $link in
freestanding)
	check freestanding "$bin" ;;
both)
	check libc "$bin"
	[ -z "$failure" ] && check freestanding "${bin}_freestanding" ;;
*)
	check libc "$bin" ;;
esac

if [ -z "$failure" ]; then
	printf '  ok    %s%s\n' "$tag" "$name"
	echo pass > "$result"
else
	report=$(printf '  FAIL  %s%-36s %s' "$tag" "$name" "$failure")
	[ $built -ne 0 ] && report=$(printf '%s\n%s' "$report" "$(printf '%s\n' "$out" | grep -m3 'error')")
	printf '%s\n' "$report"
	echo fail > "$result"
fi
exit 0
