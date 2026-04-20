import re

with open("src/render/lighting.cpp", "r") as f:
    text = f.read()

# Increase global ambient
text = text.replace('GLfloat globalAmbient[] = {0.14f, 0.14f, 0.17f, 1.0f};', 'GLfloat globalAmbient[] = {0.35f, 0.35f, 0.40f, 1.0f};')

# Make the fill light brighter to light up the walls and rear
text = text.replace('GLfloat fillAmbient[] = {0.04f, 0.04f, 0.04f, 1.0f};', 'GLfloat fillAmbient[] = {0.15f, 0.15f, 0.15f, 1.0f};')
text = text.replace('GLfloat fillDiffuse[] = {0.46f, 0.43f, 0.38f, 1.0f};', 'GLfloat fillDiffuse[] = {0.55f, 0.50f, 0.45f, 1.0f};')

# Reposition fill light to center of room instead of front
text = text.replace('GLfloat fillPos[] = {0.0f, 20.0f, 14.0f, 1.0f};', 'GLfloat fillPos[] = {0.0f, 20.0f, 0.0f, 1.0f};')

# Also in positionLights for the dynamic state:
text = text.replace('GLfloat fillAmbient[] = {0.04f, 0.04f, 0.04f, 1.0f};', 'GLfloat fillAmbient[] = {0.15f, 0.15f, 0.15f, 1.0f};')
text = text.replace('GLfloat fillDiffuse[] = {0.46f, 0.43f, 0.38f, 1.0f};', 'GLfloat fillDiffuse[] = {0.55f, 0.50f, 0.45f, 1.0f};')


with open("src/render/lighting.cpp", "w") as f:
    f.write(text)

