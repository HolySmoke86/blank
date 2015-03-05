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

# debug
CXXFLAGS += -g3 -O0

# release
#CPPFLAGS += -DNDEBUG

SOURCE_DIR := src
BUILD_DIR := build

SRC = $(wildcard $(SOURCE_DIR)/*.cpp)
OBJ = $(patsubst $(SOURCE_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC))
DEP = $(OBJ:.o=.d)
BIN = blank

all: $(BIN)

clean:
	rm -df $(OBJ) $(DEP) $(BUILD_DIR)

distclean: clean
	rm -f $(BIN)

.PHONY: all clean distclean

-include $(DEP)

$(BIN): $(OBJ)
	@echo link: $@
	@$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $^

$(BUILD_DIR)/%.o: $(SOURCE_DIR)/%.cpp | $(BUILD_DIR)
	@echo compile: $@
	@$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ -MMD -MP -MF"$(@:.o=.d)" -MT"$@" $<

$(BUILD_DIR):
	mkdir "$@"
