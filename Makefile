# Makefile: builds and runs the PC unit tests.
#   make test   -> build (if anything changed) and run the tests
#   make clean  -> delete the build folder

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -g -fsanitize=address,undefined
CPPFLAGS := -Icore

SRCS := $(wildcard core/*.cpp) $(wildcard tests/*.cpp)
HDRS := $(wildcard core/*.h) $(wildcard tests/*.h)

.PHONY: test clean

test: build/tests
	./build/tests

# Rebuild whenever any source or header changes (small project, so just recompile everything)
build/tests: $(SRCS) $(HDRS) | build
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SRCS) -o $@

build:
	mkdir -p build

clean:
	rm -rf build
