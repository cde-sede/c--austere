#!/bin/sh
# Runs tests/*.cpp; called by `make test`, which sets CXX, MODE, CXXFLAGS,
# LDFLAGS, LIB and BUILD. T selects tests by name or glob, space-separated
# (T='optional_self_assign dynamic_array_*'); empty runs all.
#
# The first lines of each test say what must happen:
#
#   // expect: exit N          builds, runs, exits with status N
#   // expect: error REGEX     does not build; diagnostics match REGEX (grep -E)
#   // expect: trap            builds, then dies by a trap (SIGILL or SIGTRAP)
#   // mode: debug             only checked in debug builds (or: release)

ulimit -c 0
mkdir -p "$BUILD/tests"
pass=0
total=0

tests=
set -f
for pattern in ${T:-*}; do
	set +f
	found=
	for test in tests/$pattern.cpp; do
		[ -f "$test" ] || continue
		tests="$tests $test"
		found=1
	done
	if [ -z "$found" ]; then
		echo "no test matches '$pattern'"
		exit 1
	fi
	set -f
done
set +f

for test in $tests; do
	name=$(basename "$test" .cpp)
	bin=$BUILD/tests/$name
	expect=$(sed -n 's|^// expect: ||p' "$test")
	kind=${expect%% *}
	arg=${expect#"$kind"}
	arg=${arg# }
	mode=$(sed -n 's|^// mode: ||p' "$test")

	if [ -n "$mode" ] && [ "$mode" != "$MODE" ]; then
		printf '  skip  %-36s %s builds only\n' "$name" "$mode"
		continue
	fi
	total=$((total + 1))

	out=$($CXX $CXXFLAGS "$test" -o "$bin" "$LIB" $LDFLAGS 2>&1)
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
		pass=$((pass + 1))
		printf '  ok    %s\n' "$name"
	else
		printf '  FAIL  %-36s %s\n' "$name" "$failure"
		[ $built -ne 0 ] && printf '%s\n' "$out" | grep -m3 'error'
	fi
done

echo "$pass/$total as expected ($CXX, $MODE)"
[ $pass -eq $total ]
