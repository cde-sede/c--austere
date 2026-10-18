#!/bin/sh
# Compiles one .cpp against ca, with the flags of the Makefile, linking the lib
# built in the same mode: NDEBUG changes the layout of optional and result, so
# a file and the lib must agree on it.
#
#   ./compile.sh file.cpp [out]      release; out defaults to file name without .cpp
#   MODE=debug ./compile.sh file.cpp
#   CXX=g++ ./compile.sh file.cpp

set -e

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
	echo "usage: [MODE=debug|release] [CXX=...] $0 file.cpp [out]" >&2
	exit 2
fi

src=$1
out=${2:-$(basename "$src" .cpp)}
root=$(cd "$(dirname "$0")" && pwd)
CXX=${CXX:-clang++}
MODE=${MODE:-release}

case $CXX in
*clang*) std=c++2c; ldextra=-nostdlib++ ;;
*)       std=c++23; ldextra= ;;
esac

case $MODE in
release) modeflags='-O2 -DNDEBUG' ;;
debug)   modeflags='-g -O1' ;;
*)       echo "MODE must be debug or release" >&2; exit 2 ;;
esac

make -s -C "$root" CXX="$CXX" MODE="$MODE"

$CXX -std=$std -nostdinc++ -fno-exceptions -fno-rtti \
	-Wall -Wextra -Wpedantic -Werror -pthread $modeflags \
	-I"$root/src/lib" \
	"$src" -o "$out" \
	"$root/build/$(basename "$CXX")-$MODE/libca.a" \
	-nostdinc++ -pthread $ldextra
