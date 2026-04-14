CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude `pkg-config --cflags opencv4`
LDFLAGS := -lGL -lGLU -lglut `pkg-config --libs opencv4`

SRC := \
	src/main.cpp \
	src/core/camera.cpp \
	src/render/lighting.cpp \
	src/render/shader.cpp \
	src/render/primitives.cpp \
	src/render/video.cpp \
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
