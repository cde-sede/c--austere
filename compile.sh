#!/bin/sh
# Compiles one .cpp against ca, with the flags of the Makefile, linking the lib
# built in the same mode: NDEBUG changes the layout of optional and result, so
# a file and the lib must agree on it.
#
#   ./compile.sh file.cpp [out]      release; out defaults to file name without .cpp
#   MODE=debug ./compile.sh file.cpp
#   CXX=g++ ./compile.sh file.cpp
#   FREESTANDING=1 ./compile.sh file.cpp
#                                    links nothing but ca: no libc, no libgcc, no crt files;
#                                    a static-pie started by ca's runtime (libca_runtime.a)

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

make -j$(nproc) -s -C "$root" CXX="$CXX" MODE="$MODE"

build="$root/build/$(basename "$CXX")-$MODE"

if [ -n "$FREESTANDING" ]; then
	set -x
	$CXX -std=$std -nostdinc++ -fno-exceptions -fno-rtti -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer \
		-Wall -Wextra -Wpedantic -Werror $modeflags \
		-I"$root/src/lib" \
		"$src" -o "$out" \
		-nostdlib -static-pie -Wl,-u,_start \
		-Wl,--start-group "$build/libca.a" "$build/libca_runtime.a" -Wl,--end-group
	exit
fi

echo $CXX -std=$std -nostdinc++ -fno-exceptions -fno-rtti -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer \
	-Wall -Wextra -Wpedantic -Werror -pthread $modeflags \
	-I"$root/src/lib" \
	"$src" -o "$out" \
	"$build/libca.a" \
	-nostdinc++ -pthread $ldextra

$CXX -std=$std -nostdinc++ -fno-exceptions -fno-rtti -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer \
	-Wall -Wextra -Wpedantic -Werror -pthread $modeflags \
	-I"$root/src/lib" \
	"$src" -o "$out" \
	"$build/libca.a" \
	-nostdinc++ -pthread $ldextra
