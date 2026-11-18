# Makefile: builds and runs the PC unit tests and the terminal version of the game.
#   make test   -> build (if anything changed) and run the tests; also builds the game, so CI catches
#                  compile errors in pc/ too
#   make play   -> build (if anything changed) and play blackjack in the terminal
#   make clean  -> delete the build folder (this also resets the PC's saved bankroll)

CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -Wshadow -Wconversion -g -fsanitize=address,undefined
CPPFLAGS := -Icore -Ipc

# the tests: the core, the tests, and the PC's FileStorage (everything in pc/ except main.cpp)
SRCS := $(wildcard core/*.cpp) $(wildcard tests/*.cpp) pc/file_storage.cpp
HDRS := $(wildcard core/*.h) $(wildcard tests/*.h) $(wildcard pc/*.h)

# the terminal game: the same core, plus the PC frontend in pc/ (no tests)
PLAY_SRCS := $(wildcard core/*.cpp) $(wildcard pc/*.cpp)
PLAY_HDRS := $(wildcard core/*.h) $(wildcard pc/*.h)

.PHONY: test play clean

test: build/tests build/blackjack
	./build/tests

play: build/blackjack
	./build/blackjack

# Rebuild whenever any source or header changes (small project, so just recompile everything)
build/tests: $(SRCS) $(HDRS) | build
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(SRCS) -o $@

build/blackjack: $(PLAY_SRCS) $(PLAY_HDRS) | build
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(PLAY_SRCS) -o $@

build:
	mkdir -p build

clean:
	rm -rf build
