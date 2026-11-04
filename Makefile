# ca - build.
#
#   make                 build build/<cxx>-<mode>/libca.a (clang++, debug)
#   make test            build, then run tests/*.cpp, each its own target (-j runs them in
#                        parallel; see tooling/test.sh), then count them (tooling/test_summary.sh)
#   make test T=NAME     run only tests/NAME.cpp; globs and several names work:
#                        T='optional_self_assign dynamic_array_*'
#   make includes        regenerate the src/lib/ca/ include facade (`#include "ca/print.hpp"`;
#                        see tooling/gen_includes.py); every build runs it
#   make leaks           no system/std header reachable from a header, no symbol
#                        outside ca in the library (see tooling/leaks.sh)
#   make matrix          facade check, leaks + test with clang++ and g++, debug and release (T too);
#                        the four are independent (matrix-g++-release, ...), so -j runs them and
#                        their tests at once, lines tagged with the configuration
#   make bench           benchmarks against std (see bench/run.sh); B=path (or ring, ...) picks
#                        bench/path.cpp instead of bench/dynamic_array.cpp
#   make bench-doc       run every benchmark a header quotes (--BENCH blocks) with clang++ and
#                        g++, rewrite those tables in the headers, regenerate doc/ (autodoc
#                        --bench); B='hashmap map' runs only those
#   make syscalls        regenerate the ca::syscall table of this machine
#                        (see tooling/gen_syscalls.py)
#   make unicode         regenerate the ca::unicode character database and the classes of
#                        ca::regex from the running Python and the UCD of its Unicode version
#                        (see tooling/gen_unicode.py)
#   make gen             run every generator, in dependency order: traits, syscall table,
#                        unicode tables, include facade (derived from the tree), str_python
#                        and regex_python tests, doc (md and html)
#   make clean           remove objects (.o) and dependency files (.d) of every build, and the
#                        generators' scratch (downloads, bytecode: tooling/<gen>.py --clean)
#   make fclean          clean, then remove build/ entirely and everything the generators
#                        wrote (--fclean: traits, facade, syscall table, unicode tables,
#                        str_python and regex_python tests, doc); make gen brings them back
#
#   make CXX=g++ MODE=release
#   make MODE=release FRAME_POINTERS=1   keep frame pointers, for full trap backtraces
#                        (ca::trace::backtrace); on by default in debug; make clean after
#                        changing it, objects are not rebuilt for it
#
# Sources are every .cpp under src/, found recursively. Each compiles to its
# own .o under the build directory, mirroring the source tree. -MMD -MP emit a
# .d per object listing the headers it includes, so touching a header rebuilds
# exactly the objects that use it, and deleting one does not break the build.
#
# Debug builds use -O1: the compile-time check of unchecked accesses relies on
# the optimizer proving them.

ifeq ($(origin CXX),default)
CXX := clang++
endif
MODE ?= debug

ifneq ($(findstring clang,$(CXX)),)
STD ?= c++2c
else
STD ?= c++23
endif

CPPFLAGS := -Isrc/lib -MMD -MP
CXXFLAGS := -std=$(STD) -nostdinc++ -fno-exceptions -fno-rtti -Wall -Wextra -Wpedantic -Werror -pthread
LDFLAGS  := -nostdinc++ -pthread
ifneq ($(findstring clang,$(CXX)),)
LDFLAGS  += -nostdlib++
endif

ifeq ($(MODE),release)
CXXFLAGS += -O2 -DNDEBUG
FRAME_POINTERS ?= 0
else ifeq ($(MODE),debug)
CXXFLAGS += -g -O1
FRAME_POINTERS ?= 1
else
$(error MODE must be debug or release)
endif

ifeq ($(FRAME_POINTERS),1)
CXXFLAGS += -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer
endif

GENERATORS := gen_traits gen_syscalls gen_unicode gen_float_tables gen_includes gen_str_test gen_regex_test autodoc

BUILD := build/$(notdir $(CXX))-$(MODE)
LIB   := $(BUILD)/libca.a
SRCS  := $(shell find src/lib -type f -name '*.cpp')
OBJS  := $(SRCS:%.cpp=$(BUILD)/%.o)

RUNTIME      := $(BUILD)/libca_runtime.a
RUNTIME_SRCS := $(shell find src/runtime -type f -name '*.cpp')
RUNTIME_OBJS := $(RUNTIME_SRCS:%.cpp=$(BUILD)/%.o)

DEPS  := $(OBJS:.o=.d) $(RUNTIME_OBJS:.o=.d)

TEST_MATCHES := $(foreach p,$(or $(T),*),$(or $(wildcard tests/$(p).cpp),$(error no test matches '$(p)')))
TEST_NAMES   := $(sort $(basename $(notdir $(TEST_MATCHES))))
TEST_RESULTS := $(TEST_NAMES:%=$(BUILD)/tests/%.result)

CONFIGS := clang++-debug clang++-release g++-debug g++-release

.PHONY: all includes test leaks matrix matrix-includes $(CONFIGS:%=matrix-%) bench bench-doc syscalls unicode gen clean fclean FORCE

all: includes $(LIB) $(RUNTIME)

includes:
ifeq ($(MATRIX),)
	@python3 tooling/gen_includes.py
endif

$(LIB): $(OBJS)
	@mkdir -p $(@D)
	rm -f $@
	$(AR) rcs $@ $^

$(RUNTIME): $(RUNTIME_OBJS)
	@mkdir -p $(@D)
	rm -f $@
	$(AR) rcs $@ $^

$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

test: $(TEST_RESULTS)
	@CXX='$(CXX)' MODE='$(MODE)' BUILD='$(BUILD)' tooling/test_summary.sh $(TEST_NAMES)

$(BUILD)/tests/%.result: tests/%.cpp $(LIB) $(RUNTIME) FORCE | includes
	@CXX='$(CXX)' MODE='$(MODE)' CXXFLAGS='$(CPPFLAGS) $(CXXFLAGS)' LDFLAGS='$(LDFLAGS)' \
		LIB='$(LIB)' RUNTIME='$(RUNTIME)' BUILD='$(BUILD)' TAG='$(TAG)' tooling/test.sh $<

FORCE:

leaks: $(LIB)
	@CXX='$(CXX)' CXXFLAGS='$(CPPFLAGS) $(CXXFLAGS)' LIB='$(LIB)' tooling/leaks.sh

matrix: $(CONFIGS:%=matrix-%)

matrix-includes:
	@python3 tooling/gen_includes.py --check

$(CONFIGS:%=matrix-%): matrix-%: matrix-includes
	@$(MAKE) --no-print-directory MATRIX=1 CXX=$(word 1,$(subst -, ,$*)) MODE=$(word 2,$(subst -, ,$*)) \
		TAG='$(subst -, ,$*)' T='$(T)' leaks test

bench:
	@bench/run.sh -c $(CXX) -b $(or $(B),dynamic_array)

bench-doc:
	@python3 tooling/autodoc.py --bench $(B)
	@python3 tooling/autodoc.py -f html

syscalls:
	python3 tooling/gen_syscalls.py

unicode:
	python3 tooling/gen_unicode.py

gen:
	@for g in $(GENERATORS); do python3 tooling/$$g.py || exit 1; done
	@python3 tooling/autodoc.py -f html

clean:
	@if [ -d build ]; then find build \( -name '*.o' -o -name '*.d' \) -type f -delete; find build -mindepth 1 -type d -empty -delete; fi
	@for g in $(GENERATORS); do python3 tooling/$$g.py --clean || exit 1; done

fclean: clean
	rm -rf build
	@for g in $(GENERATORS); do python3 tooling/$$g.py --fclean || exit 1; done

-include $(DEPS)
