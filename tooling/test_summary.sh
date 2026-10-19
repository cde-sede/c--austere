#!/bin/sh
# Counts the results tooling/test.sh wrote for the tests named as arguments;
# called by `make test` after every test ran, with CXX, MODE and BUILD set.
# Fails unless every test that ran passed.
#
#   tooling/test_summary.sh NAME...

pass=0
total=0
for name in "$@"; do
	result=$(cat "$BUILD/tests/$name.result" 2>/dev/null || echo fail)
	case $result in
	skip) continue ;;
	pass) pass=$((pass + 1)) ;;
	esac
	total=$((total + 1))
done

echo "$pass/$total as expected ($CXX, $MODE)"
[ $pass -eq $total ]
