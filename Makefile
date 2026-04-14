CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
LDFLAGS := -lGL -lGLU -lglut

SRC := \
	src/main.cpp \
	src/core/camera.cpp \
	src/render/lighting.cpp \
	src/render/shader.cpp \
	src/render/primitives.cpp \
	src/scene/room.cpp \
	src/scene/stage.cpp \
	src/scene/seats.cpp

OUT := th

.PHONY: all clean run

all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT) $(LDFLAGS)

run: $(OUT)
	./$(OUT)

clean:
	rm -f $(OUT)
