with open("Makefile", "w") as f:
    f.write("CXX = g++\n")
    f.write("CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Iinclude $(shell pkg-config --cflags opencv4)\n")
    f.write("LDFLAGS = -lGL -lGLU -lglut $(shell pkg-config --libs opencv4)\n\n")
    f.write("SRCS = src/main.cpp src/core/camera.cpp src/render/lighting.cpp src/render/shader.cpp src/render/primitives.cpp src/render/video.cpp src/scene/room.cpp src/scene/stage.cpp src/scene/seats.cpp\n\n")
    f.write("TARGET = th\n\n")
    f.write("all: $(TARGET)\n\n")
    f.write("$(TARGET): $(SRCS)\n")
    f.write("\t$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)\n\n")
    f.write("clean:\n")
    f.write("\trm -f $(TARGET)\n\n")
    f.write(".PHONY: all clean\n")

