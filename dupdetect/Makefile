CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2 -Isrc
BUILD    := build

ifeq ($(OS),Windows_NT)
  EXE := .exe
endif

APP_SRCS := $(filter-out src/main.cpp,$(wildcard src/*.cpp))
HEADERS  := $(wildcard src/*.h)
TEST_SRCS := $(wildcard tests/*.cpp)
TEST_HDRS := $(wildcard tests/*.h)
APP      := $(BUILD)/dupdetect$(EXE)
TEST_BIN := $(BUILD)/run_tests$(EXE)

all: $(APP)

$(APP): $(APP_SRCS) src/main.cpp $(HEADERS)
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $(APP_SRCS) src/main.cpp -o $@

$(TEST_BIN): $(APP_SRCS) $(TEST_SRCS) $(HEADERS) $(TEST_HDRS)
	@mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) -Itests $(APP_SRCS) $(TEST_SRCS) -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

clean:
	rm -rf $(BUILD)

.PHONY: all test clean
