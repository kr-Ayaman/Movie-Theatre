with open("src/scene/room.cpp", "r") as f:
    text = f.read()

text = text.replace("float doorZ = 10.0f; // Align both precisely in front of each other", "float doorZ = -5.0f; // Moved near the front stage")
text = text.replace("float wallX = isLeftWall ? -16.45f : 16.45f;", "float wallX = isLeftWall ? -16.6f : 16.6f;")

with open("src/scene/room.cpp", "w") as f:
    f.write(text)

