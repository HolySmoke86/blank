CXX = g++ --std=c++11
LDXX = g++
CPPCHECK = cppcheck -q --std=c++11 \
	--enable=warning,style,performance,portability,unusedFunction,missingInclude \
	--error-exitcode=1

LIBS = sdl2 SDL2_image SDL2_net SDL2_ttf glew openal freealut zlib

PKGFLAGS := $(shell pkg-config --cflags $(LIBS))
PKGLIBS := $(shell pkg-config --libs $(LIBS))
TESTFLAGS := $(shell pkg-config --cflags cppunit)
TESTLIBS := $(shell pkg-config --libs cppunit)

CPPFLAGS ?=
CPPFLAGS += $(PKGFLAGS)
CXXFLAGS ?=
CXXFLAGS += -Wall -Wextra -Werror
#CXXFLAGS += -march=native
LDXXFLAGS ?=
LDXXFLAGS += $(PKGLIBS)

# source
SOURCE_DIR := src
TEST_SRC_DIR := tst

# build configurations
# cover:
#   coverage reporting
#   for use with gcov
# debug:
#   unoptimized and maximally annotated
#   for use with gdb
# profile:
#   somewhat optimized and maximally annotated
#   for use with valgrind
# release:
#   optimized, without debugging instructions and minimally
#   annotated (mainly for stack traces)
#   for use with people
# tests:
#   same flags as release, but with main replaced by cppunit
#   test runner and tests (from tst dir) built in

COVER_FLAGS = -g -O0 --coverage -I$(SOURCE_DIR) $(TESTFLAGS) -DBLANK_SUFFIX=.cover
DEBUG_FLAGS = -g3 -O0
PROFILE_FLAGS = -DNDEBUG -O1 -g3 -DBLANK_PROFILING
RELEASE_FLAGS = -DNDEBUG -O2 -g1
TEST_FLAGS = -g -O2 -I$(SOURCE_DIR) $(TESTFLAGS) -DBLANK_SUFFIX=.test

# destination
COVER_DIR := build/cover
DEBUG_DIR := build/debug
PROFILE_DIR := build/profile
RELEASE_DIR := build/release
TEST_DIR := build/test

DIR := $(RELEASE_DIR) $(COVER_DIR) $(DEBUG_DIR) $(PROFILE_DIR) $(TEST_DIR) build

ASSET_DIR := assets
ASSET_DEP := $(ASSET_DIR)/.git

LIB_SRC := $(wildcard $(SOURCE_DIR)/*/*.cpp)
BIN_SRC := $(wildcard $(SOURCE_DIR)/*.cpp)
SRC := $(LIB_SRC) $(BIN_SRC)
TEST_BIN_SRC := $(wildcard $(TEST_SRC_DIR)/*.cpp)
TEST_LIB_SRC := $(wildcard $(TEST_SRC_DIR)/*/*.cpp)
TEST_SRC := $(TEST_LIB_SRC) $(TEST_BIN_SRC)

COVER_OBJ := $(patsubst $(TEST_SRC_DIR)/%.cpp, $(COVER_DIR)/%.o, $(TEST_SRC)) $(patsubst $(SOURCE_DIR)/%.cpp, $(COVER_DIR)/src/%.o, $(LIB_SRC))
COVER_DEP := $(COVER_OBJ:.o=.d)
COVER_BIN := blank.cover test.cover

DEBUG_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(SRC))
DEBUG_LIB_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(LIB_SRC))
DEBUG_DEP := $(DEBUG_OBJ:.o=.d)
DEBUG_BIN := blank.debug

PROFILE_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(PROFILE_DIR)/%.o, $(SRC))
PROFILE_LIB_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(PROFILE_DIR)/%.o, $(LIB_SRC))
PROFILE_DEP := $(PROFILE_OBJ:.o=.d)
PROFILE_BIN := blank.profile generate.profile

RELEASE_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(SRC))
RELEASE_LIB_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(LIB_SRC))
RELEASE_DEP := $(RELEASE_OBJ:.o=.d)
RELEASE_BIN := blank

TEST_OBJ := $(patsubst $(TEST_SRC_DIR)/%.cpp, $(TEST_DIR)/%.o, $(TEST_SRC)) $(patsubst $(SOURCE_DIR)/%.cpp, $(TEST_DIR)/src/%.o, $(LIB_SRC))
TEST_DEP := $(TEST_OBJ:.o=.d)
TEST_BIN := blank.test test.test

OBJ := $(COVER_OBJ) $(DEBUG_OBJ) $(PROFILE_OBJ) $(RELEASE_OBJ) $(TEST_OBJ)
DEP := $(COVER_DEP) $(DEBUG_DEP) $(PROFILE_DEP) $(RELEASE_DEP) $(TEST_DEP)
BIN := $(COVER_BIN) $(DEBUG_BIN) $(PROFILE_BIN) $(RELEASE_BIN) $(TEST_BIN)

release: $(RELEASE_BIN)

info:
	@echo "CXX:  $(CXX)"
	@echo "LDXX: $(LDXX)"
	@echo
	@echo "LIBS: $(LIBS)"
	@echo
	@echo "CPPFLAGS:  $(CPPFLAGS)"
	@echo "CXXFLAGS:  $(CXXFLAGS)"
	@echo "LDXXFLAGS: $(LDXXFLAGS)"
	@echo "TESTFLAGS: $(TESTFLAGS)"
	@echo "TESTLIBS:  $(TESTLIBS)"
	@echo
	@-lsb_release -a
	@git --version
	@g++ --version

all: $(BIN)

cover: $(COVER_BIN)

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

test: $(TEST_BIN)
	@echo run: test.test
	@./test.test

coverage: $(COVER_BIN)
	@echo run: test.cover
	@./test.cover

codecov: coverage
	@echo run: codecov.io
	@bash -c 'bash <(curl -s https://codecov.io/bash) -Z'

lint:
	@echo lint: source
	@$(CPPCHECK) $(SOURCE_DIR)
	@echo lint: tests
	@$(CPPCHECK) -I $(SOURCE_DIR) $(TEST_SRC_DIR)

clean:
	rm -f $(OBJ)
	rm -f $(DEP)
	find build -type d -empty -delete

distclean: clean
	rm -f $(BIN) cachegrind.out.* callgrind.out.*
	rm -Rf build client-saves saves

.PHONY: all release cover debug profile tests run gdb cachegrind callgrind test coverage codecov lint clean distclean

-include $(DEP)


$(COVER_BIN): $(COVER_OBJ)
	@echo link: $@
	@$(LDXX) $(CXXFLAGS) $^ -o $@ $(LDXXFLAGS) $(TESTLIBS) $(COVER_FLAGS)

$(COVER_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp | $(COVER_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(COVER_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(COVER_DIR)/src/%.o: $(SOURCE_DIR)/%.cpp | $(COVER_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(COVER_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<


$(DEBUG_BIN): %.debug: $(DEBUG_DIR)/%.o $(DEBUG_LIB_OBJ)
	@echo link: $@
	@$(LDXX) $(CXXFLAGS) $^ -o $@ $(LDXXFLAGS) $(DEBUG_FLAGS)

$(DEBUG_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(DEBUG_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<


$(PROFILE_BIN): %.profile: $(PROFILE_DIR)/%.o $(PROFILE_LIB_OBJ)
	@echo link: $@
	@$(LDXX) $(CXXFLAGS) $^ -o $@ $(LDXXFLAGS) $(PROFILE_FLAGS)

$(PROFILE_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(PROFILE_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(PROFILE_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<


$(RELEASE_BIN): %: $(RELEASE_DIR)/%.o $(RELEASE_LIB_OBJ)
	@echo link: $@
	@$(LDXX) $(CXXFLAGS) $^ -o $@ $(LDXXFLAGS) $(RELEASE_FLAGS)

$(RELEASE_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(RELEASE_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<


$(TEST_BIN): $(TEST_OBJ)
	@echo link: $@
	@$(LDXX) $(CXXFLAGS) $^ -o $@ $(LDXXFLAGS) $(TESTLIBS) $(TEST_FLAGS)

$(TEST_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp | $(TEST_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(TEST_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(TEST_DIR)/src/%.o: $(SOURCE_DIR)/%.cpp | $(TEST_DIR)
	@mkdir -p "$(@D)"
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(TEST_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<


$(ASSET_DEP): .git/$(shell git symbolic-ref HEAD 2>/dev/null || echo HEAD)
	@echo fetch: assets
	@git submodule update --init >/dev/null
	@touch $@

$(DIR):
	@mkdir -p "$@"
