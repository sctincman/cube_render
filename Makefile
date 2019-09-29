CXX = g++
SDL_LIB = -L/usr/lib -lGL -lGLEW -lSDL2 -Wl,-rpath=/usr/lib
SDL_INCLUDE = -I/usr/include -DGL_GLEXT_PROTOTYPES

TINY_GLTF_INCLUDE = -Iexternal/tinygltf #-DTINYGLTF_IMPLEMENTATION -DSTB_IMAGE_IMPLEMENTATION -DSTB_IMAGE_WRITE_IMPLEMENTATION

CXXFLAGS = -Wall -c -std=c++11 $(SDL_INCLUDE) $(TINY_GLTF_INCLUDE)
LDFLAGS = $(SDL_LIB)

SRC := src
OBJ := obj
SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(OBJ)/%.o, $(SOURCES))

EXE = cube_render

DEBUG ?= 0
ifeq ($(DEBUG), 1)
    CXXFLAGS := $(CXXFLAGS) -g -DCUBE_DEBUG
    EXE := $(EXE)-debug
endif

all: $(EXE)


$(EXE): $(OBJECTS)

	$(CXX) $(LDFLAGS) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.cpp $(OBJ)

	$(CXX) $(CXXFLAGS) $< -o $@

$(OBJ):

	mkdir -p $(OBJ)

clean:

	rm -r $(OBJ)
