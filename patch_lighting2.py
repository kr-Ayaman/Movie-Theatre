import re

with open('src/render/lighting.cpp', 'r') as f:
    text = f.read()

# Make the key light wider and softer
text = text.replace('glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 36.0f);', 'glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 85.0f);')
text = text.replace('glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 22.0f);', 'glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.5f);')

with open('src/render/lighting.cpp', 'w') as f:
    f.write(text)

