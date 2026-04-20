makefile_content = """CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -Iinclude $$(pkg-config --cflags opencv4)
LDFLAGS = -lGL -lGLU -lglut $$(pkg-config --libs opencv4)

SRCS = src/main.cpp \\
       src/core/camera.cpp \\
       src/render/lighting.cpp \\
       src/render/shader.cpp \\
       src/render/primitives.cpp \\
       src/render/video.cpp \\
       src/scene/room.cpp \\
       src/scene/stage.cpp \\
       src/scene/seats.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = th

all: $(TARGET)

$(TARGET): $(OBJS)
\t$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
\t$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
\trm -f $(OBJS) $(TARGET)

.PHONY: all clean
"""

with open('Makefile', 'w') as f:
    f.write(makefile_content)
