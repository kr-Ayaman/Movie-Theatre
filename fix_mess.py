import os

with open("src/scene/room.cpp", "r") as f:
    r_text = f.read()

r_text = r_text.replace('void drawSimpleWalls() {\n    setSceneShaderEffect(kSceneShaderEffectDefault);\n    setMaterial(0.73f, 0.74f, 0.76f, 9.0f, 0.04f);', 'void drawSimpleWalls() {\n    setSceneShaderEffect(kSceneShaderEffectBrick);\n    setMaterial(0.73f, 0.74f, 0.76f, 9.0f, 0.04f);')

with open("src/scene/room.cpp", "w") as f:
    f.write(r_text)

with open("src/scene/seats.cpp", "r") as f:
    s_text = f.read()

s_text = s_text.replace('const int rows = 15;', 'const int rows = 14;')
s_text = s_text.replace('for (int row = 0; row < 15; ++row)', 'for (int row = 0; row < 14; ++row)')
s_text = s_text.replace('if (row == 14)', 'if (row == 13)')

with open("src/scene/seats.cpp", "w") as f:
    f.write(s_text)

