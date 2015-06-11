CXX = g++ --std=c++11
LDXX = g++

LIBS = sdl2 SDL2_image glew

PKGFLAGS := $(shell pkg-config --cflags $(LIBS))
PKGLIBS := $(shell pkg-config --libs $(LIBS))
TESTFLAGS := $(shell pkg-config --cflags cppunit)
TESTLIBS := $(shell pkg-config --libs cppunit)

CPPFLAGS ?=
CPPFLAGS += $(PKGFLAGS)
CXXFLAGS ?=
CXXFLAGS += -Wall
LDXXFLAGS ?=
LDXXFLAGS += $(PKGLIBS)

DEBUG_FLAGS = -g3 -O0
PROFILE_FLAGS = -DNDEBUG -O1 -g3
RELEASE_FLAGS = -DNDEBUG -O2
TEST_FLAGS = -g -O2 -I./src $(TESTFLAGS)

SOURCE_DIR := src
TEST_SRC_DIR := tst
DEBUG_DIR := build/debug
PROFILE_DIR := build/profile
RELEASE_DIR := build/release
TEST_DIR := build/test
DIR := $(RELEASE_DIR) $(DEBUG_DIR) $(PROFILE_DIR) $(TEST_DIR) build

LIB_SRC := $(wildcard $(SOURCE_DIR)/*/*.cpp)
BIN_SRC := $(wildcard $(SOURCE_DIR)/*.cpp)
SRC := $(LIB_SRC) $(BIN_SRC)
TEST_SRC := $(wildcard $(TEST_SRC_DIR)/*.cpp) $(wildcard $(TEST_SRC_DIR)/*/*.cpp)
RELEASE_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(SRC))
DEBUG_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(SRC))
PROFILE_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(PROFILE_DIR)/%.o, $(SRC))
TEST_OBJ := $(patsubst $(TEST_SRC_DIR)/%.cpp, $(TEST_DIR)/%.o, $(TEST_SRC)) $(patsubst $(SOURCE_DIR)/%.cpp, $(TEST_DIR)/src/%.o, $(LIB_SRC))
RELEASE_DEP := $(RELEASE_OBJ:.o=.d)
DEBUG_DEP := $(DEBUG_OBJ:.o=.d)
PROFILE_DEP := $(PROFILE_OBJ:.o=.d)
TEST_DEP := $(TEST_OBJ:.o=.d)
RELEASE_BIN := blank
DEBUG_BIN := blank.debug
PROFILE_BIN := blank.profile
TEST_BIN := blank.test
OBJ := $(RELEASE_OBJ) $(DEBUG_OBJ) $(PROFILE_OBJ) $(TEST_OBJ)
DEP := $(RELEASE_DEP) $(DEBUG_DEP) $(PROFILE_DEP) $(TEST_DEP)
BIN := $(RELEASE_BIN) $(DEBUG_BIN) $(PROFILE_BIN) $(TEST_BIN)

release: $(RELEASE_BIN)

all: $(BIN)

debug: $(DEBUG_BIN)

profile: $(PROFILE_BIN)

tests: $(TEST_BIN)

run: blank
	./blank

gdb: blank.debug
	gdb ./blank.debug

cachegrind: blank.profile
	valgrind ./blank.profile

callgrind: blank.profile
	valgrind --tool=callgrind \
		--branch-sim=yes --cacheuse=yes --cache-sim=yes \
		--collect-bus=yes --collect-systime=yes --collect-jumps=yes \
		--dump-instr=yes --simulate-hwpref=yes --simulate-wb=yes \
		./blank.profile -n 128 -t 16 --no-keyboard --no-mouse -d --no-vsync

test: blank.test
	./blank.test

clean:
	rm -df $(OBJ) $(DEP) $(DIR)

distclean: clean
	rm -f $(BIN) cachegrind.out.* callgrind.out.*

.PHONY: all release debug profile tests run gdb cachegrind callgrind test clean distclean

-include $(DEP)

$(RELEASE_BIN): $(RELEASE_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(RELEASE_FLAGS) $^

$(DEBUG_BIN): $(DEBUG_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(DEBUG_FLAGS) $^

$(PROFILE_BIN): $(PROFILE_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(PROFILE_FLAGS) $^

$(TEST_BIN): $(TEST_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(TESTLIBS) $(TEST_FLAGS) $^

$(RELEASE_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(RELEASE_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(DEBUG_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(DEBUG_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(PROFILE_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(PROFILE_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(PROFILE_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(TEST_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp | $(TEST_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(TEST_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(TEST_DIR)/src/%.o: $(SOURCE_DIR)/%.cpp | $(TEST_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(TEST_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(DIR):
	@mkdir -p "$@"
