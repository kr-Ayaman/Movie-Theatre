import re
with open('src/render/lighting.cpp', 'r') as f:
    text = f.read()

# Make GL_LIGHT0 a directional light pointing straight down instead of a spot light
text = text.replace('GLfloat keyPos[] = {0.0f, 20.0f, -8.0f, 1.0f};', 'GLfloat keyPos[] = {0.0f, 1.0f, 0.0f, 0.0f}; // Directional light from above')
text = text.replace('GLfloat keyDir[] = {0.0f, -1.0f, -0.25f};', '// No spot direction for directional light')
text = text.replace('glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, keyDir);', '//')

# Also turn off the GL_SPOT_CUTOFF from initLighting since it is now directional
init_find = text.find('glLightf(GL_LIGHT0, GL_SPOT_CUTOFF,')
if init_find != -1:
    end_find = text.find(';', init_find)
    text = text[:init_find] + '// ' + text[init_find:]

init_exp = text.find('glLightf(GL_LIGHT0, GL_SPOT_EXPONENT,')
if init_exp != -1:
    end_find = text.find(';', init_exp)
    text = text[:init_exp] + '// ' + text[init_exp:]

with open('src/render/lighting.cpp', 'w') as f:
    f.write(text)
