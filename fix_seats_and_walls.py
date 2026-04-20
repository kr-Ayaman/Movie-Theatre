with open("src/scene/seats.cpp", "r") as f:
    text = f.read()

# Increase rows to 15
text = text.replace('const int rows = 12;', 'const int rows = 15;')
text = text.replace('for (int row = 0; row < 12; ++row)', 'for (int row = 0; row < 15; ++row)')
text = text.replace('if (row == 11)', 'if (row == 14)')

with open("src/scene/seats.cpp", "w") as f:
    f.write(text)

with open("src/scene/room.cpp", "r") as f:
    r_text = f.read()
    
# Change the effect to default or less abrasive tiles
r_text = r_text.replace('setSceneShaderEffect(kSceneShaderEffectBrick);', 'setSceneShaderEffect(kSceneShaderEffectDefault);')

with open("src/scene/room.cpp", "w") as f:
    f.write(r_text)

