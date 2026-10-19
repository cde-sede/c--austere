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
#   // link: freestanding      linked without libc: static, ca's own _start and runtime
#                              (RUNTIME, built from src/runtime/), no startup files

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

link=$(sed -n 's|^// link: ||p' "$test")
if [ "$link" = freestanding ]; then
	out=$($CXX $CXXFLAGS "$test" -o "$bin" -nostdlib -static -Wl,-u,_start \
		-Wl,--start-group "$LIB" "$RUNTIME" -Wl,--end-group 2>&1)
else
	out=$($CXX $CXXFLAGS "$test" -o "$bin" "$LIB" $LDFLAGS 2>&1)
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
		status=$("$bin" >/dev/null 2>&1; echo $?)
		if [ "$kind" = exit ] && [ "$status" != "$arg" ]; then
			failure="expected exit $arg, got status $status"
		elif [ "$kind" = trap ] && [ "$status" != 132 ] && [ "$status" != 133 ]; then
			failure="expected a trap, got status $status"
		fi
	fi ;;
*)
	failure='no "// expect:" line' ;;
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
