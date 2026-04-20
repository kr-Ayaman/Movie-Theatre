import re

with open('src/main.cpp', 'r') as f:
    text = f.read()

# Change perspective to ortho for the light projection
orig_proj = 'gluPerspective(60.0f, (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT, 2.0f, 80.0f);'
new_proj = 'glOrtho(-30.0f, 30.0f, -40.0f, 40.0f, 1.0f, 60.0f);'
text = text.replace(orig_proj, new_proj)

# Change lookAt for the light to look straight down
orig_lookat = '''gluLookAt(lightPos[0], lightPos[1], lightPos[2],
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);'''
new_lookat = '''gluLookAt(0.0f, 25.0f, 0.0f, // high above center
              0.0f, 0.0f, 0.0f,
              0.0f, 0.0f, -1.0f); // Up vector points -z since we look -y'''
text = text.replace(orig_lookat, new_lookat)

with open('src/main.cpp', 'w') as f:
    f.write(text)

