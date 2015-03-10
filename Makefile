CXX = g++ --std=c++11
LDXX = g++

LIBS = sdl2 SDL2_image glew

PKGFLAGS := $(shell pkg-config --cflags $(LIBS))
PKGLIBS := $(shell pkg-config --libs $(LIBS))

CPPFLAGS ?=
CPPFLAGS += $(PKGFLAGS)
CXXFLAGS ?=
CXXFLAGS += -Wall
LDXXFLAGS ?=
LDXXFLAGS += $(PKGLIBS)

DEBUG_FLAGS = -g3 -O0
RELEASE_FLAGS = -DNDEBUG -O2

SOURCE_DIR := src
DEBUG_DIR := build/debug
RELEASE_DIR := build/release
DIR := $(RELEASE_DIR) $(DEBUG_DIR) build

SRC := $(wildcard $(SOURCE_DIR)/*.cpp)
RELEASE_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(RELEASE_DIR)/%.o, $(SRC))
DEBUG_OBJ := $(patsubst $(SOURCE_DIR)/%.cpp, $(DEBUG_DIR)/%.o, $(SRC))
RELEASE_DEP := $(RELEASE_OBJ:.o=.d)
DEBUG_DEP := $(DEBUG_OBJ:.o=.d)
RELEASE_BIN := blank
DEBUG_BIN := blank.debug
OBJ := $(RELEASE_OBJ) $(DEBUG_OBJ)
DEP := $(RELEASE_DEP) $(DEBUG_DEP)
BIN := $(RELEASE_BIN) $(DEBUG_BIN)

all: $(BIN)

release: $(RELEASE_BIN)

debug: $(DEBUG_BIN)

run: blank
	./blank

gdb: blank.debug
	gdb ./blank.debug

clean:
	rm -df $(OBJ) $(DEP) $(DIR)

distclean: clean
	rm -f $(BIN)

.PHONY: all release debug run gdb clean distclean

-include $(DEP)

$(RELEASE_BIN): $(RELEASE_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(RELEASE_FLAGS) $^

$(DEBUG_BIN): $(DEBUG_OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $(DEBUG_FLAGS) $^

$(RELEASE_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(RELEASE_DIR)
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(RELEASE_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(DEBUG_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(DEBUG_DIR)
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_FLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(DIR):
	@mkdir -p "$@"
