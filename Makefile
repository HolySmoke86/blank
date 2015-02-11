CXX = g++ --std=c++11
LDXX = g++

LIBS = sdl2 glew

PKGFLAGS := $(shell pkg-config --cflags $(LIBS))
PKGLIBS := $(shell pkg-config --libs $(LIBS))

CPPFLAGS ?=
CPPFLAGS += $(PKGFLAGS)
CXXFLAGS ?=
CXXFLAGS += -Wall
LDXXFLAGS ?=
LDXXFLAGS += $(PKGLIBS)

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)
DEP = $(SRC:.cpp=.d)
BIN = blank

all: $(BIN)

clean:
	rm -f $(BIN) $(OBJ) $(DEP)

.PHONY: all clean

-include $(DEP)

$(BIN): $(OBJ)
	$(LDXX) -o $@ $(CXXFLAGS) $(LDXXFLAGS) $^

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -o $@ -MMD -MP -MF"$*".d -MT"$@" $<
