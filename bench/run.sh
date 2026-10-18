#!/bin/sh
# Runs bench/NAME.cpp (default dynamic_array) on its std equivalent and on ca
# from each given lib directory (default src/lib), then prints them side by side.
#
#   bench/run.sh [-c CXX] [-b NAME] [LIBDIR...]
#
# Columns are user-space cycles (min of 9 runs), the stable figure on a laptop;
# instructions and wall time are in build/bench/*.txt. For quick A/B runs while
# working; the tables in the headers come from `make bench-doc` (autodoc --bench).

cxx=clang++
bench=dynamic_array
while :; do
	case $1 in
	-c) cxx=$2; shift 2 ;;
	-b) bench=$2; shift 2 ;;
	*) break ;;
	esac
done
[ $# -eq 0 ] && set -- src/lib
std=c++2c
case $cxx in g++*) std=c++23 ;; esac

out=build/bench/$bench/$(basename "$cxx")
mkdir -p "$out"
flags="-std=$std -O2 -DNDEBUG -fno-exceptions -fno-rtti"

$cxx $flags -DUSE_STD "bench/$bench.cpp" -o "$out/std" || exit 1
names=std
i=0
for lib in "$@"; do
	i=$((i + 1))
	$cxx $flags -I"$lib" "bench/$bench.cpp" $(find "$lib" -name '*.cpp') -o "$out/ca$i" || exit 1
	names="$names ca$i"
done

for name in $names; do
	taskset -c 2 "$out/$name" > "$out/$name.txt" || exit 1
done

echo "$bench, $cxx, cycles; ca1..: $*"
awk -v names="$names" -v dir="$out" '
	BEGIN {
		n = split(names, cols, " ")
		for (c = 1; c <= n; ++c) {
			file = dir "/" cols[c] ".txt"
			row = 0
			while ((getline line < file) > 0) {
				if (row++ == 0)
					continue
				split(line, f, "\t")
				key[row] = f[1] == "" ? f[2] : f[1] ": " f[2]
				val[row, c] = f[3]
				if (length(key[row]) > width)
					width = length(key[row])
			}
			rows = row
		}
		printf "%-" width "s", "case"
		for (c = 1; c <= n; ++c)
			printf " %13s", cols[c]
		printf "\n"
		for (r = 2; r <= rows; ++r) {
			printf "%-" width "s", key[r]
			for (c = 1; c <= n; ++c)
				printf " %13s", val[r, c]
			printf "\n"
		}
	}'
