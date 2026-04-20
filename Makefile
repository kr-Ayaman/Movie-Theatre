CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Iinclude $(shell pkg-config --cflags opencv4)
LDFLAGS = -lGL -lGLU -lglut $(shell pkg-config --libs opencv4)

SRCS = src/main.cpp src/core/camera.cpp src/render/lighting.cpp src/render/shader.cpp src/render/primitives.cpp src/render/video.cpp src/scene/room.cpp src/scene/stage.cpp src/scene/seats.cpp

TARGET = th

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
