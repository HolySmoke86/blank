CXX = g++ --std=c++11
LDXX = g++

LIBS = sdl2 SDL2_image SDL2_net SDL2_ttf glew openal freealut zlib

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
PROFILE_FLAGS = -DNDEBUG -O1 -g3 -DBLANK_PROFILING
RELEASE_FLAGS = -DNDEBUG -O2 -g1
TEST_FLAGS = -g -O2 -I./src $(TESTFLAGS)

SOURCE_DIR := src
TEST_SRC_DIR := tst
DEBUG_DIR := build/debug
PROFILE_DIR := build/profile
RELEASE_DIR := build/release
TEST_DIR := build/test
DIR := $(RELEASE_DIR) $(DEBUG_DIR) $(PROFILE_DIR) $(TEST_DIR) build

ASSET_DIR := assets
ASSET_DEP := $(ASSET_DIR)/.git

LIB_SRC := $(wildcard $(SOURCE_DIR)/*/*.cpp)
BIN_SRC := $(wildcard $(SOURCE_DIR)/*.cpp)
SRC := $(LIB_SRC) $(BIN_SRC)
TEST_SRC := $(wildcard $(TEST_SRC_DIR)/*.cpp) $(wildcard $(TEST_SRC_DIR)/*/*.cpp)
RELEASE_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(SRC))
RELEASE_LIB_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(LIB_SRC))
DEBUG_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(SRC))
DEBUG_LIB_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(LIB_SRC))
PROFILE_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(PROFILE_DIR)/%.o, $(SRC))
PROFILE_LIB_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(PROFILE_DIR)/%.o, $(LIB_SRC))
TEST_OBJ := $(patsubst $(TEST_SRC_DIR)/%.cpp, $(TEST_DIR)/%.o, $(TEST_SRC)) $(patsubst $(SOURCE_DIR)/%.cpp, $(TEST_DIR)/src/%.o, $(LIB_SRC))
RELEASE_DEP := $(RELEASE_OBJ:.o=.d)
DEBUG_DEP := $(DEBUG_OBJ:.o=.d)
PROFILE_DEP := $(PROFILE_OBJ:.o=.d)
TEST_DEP := $(TEST_OBJ:.o=.d)
RELEASE_BIN := blank
DEBUG_BIN := blank.debug
PROFILE_BIN := blank.profile generate.profile
TEST_BIN := blank.test
OBJ := $(RELEASE_OBJ) $(DEBUG_OBJ) $(PROFILE_OBJ) $(TEST_OBJ)
DEP := $(RELEASE_DEP) $(DEBUG_DEP) $(PROFILE_DEP) $(TEST_DEP)
BIN := $(RELEASE_BIN) $(DEBUG_BIN) $(PROFILE_BIN) $(TEST_BIN)

release: $(RELEASE_BIN)

all: $(BIN)

debug: $(DEBUG_BIN)

profile: $(PROFILE_BIN)

tests: $(TEST_BIN)

run: $(ASSET_DEP) blank
	./blank --save-path saves/

server: $(ASSET_DEP) blank
	./blank --server --save-path saves/

client: $(ASSET_DEP) blank
	./blank --client --save-path saves/

gdb: $(ASSET_DEP) blank.debug
	gdb ./blank.debug

cachegrind: $(ASSET_DEP) blank.profile
	valgrind ./blank.profile --save-path saves/

callgrind: $(ASSET_DEP) blank.profile
	valgrind --tool=callgrind \
		--collect-atstart=no --toggle-collect="blank::Runtime::RunStandalone()" \
		--branch-sim=yes --cacheuse=yes --cache-sim=yes \
		--collect-bus=yes --collect-systime=yes --collect-jumps=yes \
		--dump-instr=yes --simulate-hwpref=yes --simulate-wb=yes \
		./blank.profile -n 256 -t 16 --no-keyboard --no-mouse -d --no-vsync --save-path saves/

test: blank.test
	./blank.test

clean:
	rm -df $(OBJ) $(DEP) $(DIR)

distclean: clean
	rm -f $(BIN) cachegrind.out.* callgrind.out.*
	rm -Rf build client-saves saves

.PHONY: all release debug profile tests run gdb cachegrind callgrind test clean distclean

-include $(DEP)

$(RELEASE_BIN): %: $(RELEASE_DIR)/%.o $(RELEASE_LIB_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(RELEASE_FLAGS) $^

$(DEBUG_BIN): %.debug: $(DEBUG_DIR)/%.o $(DEBUG_LIB_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(DEBUG_FLAGS) $^

$(PROFILE_BIN): %.profile: $(PROFILE_DIR)/%.o $(PROFILE_LIB_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(PROFILE_FLAGS) $^

$(TEST_BIN): $(TEST_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(TESTLIBS) $(TEST_FLAGS) $^

$(ASSET_DEP): .git/$(shell git symbolic-ref HEAD)
	@git submodule update --init >/dev/null
	@touch $@

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
