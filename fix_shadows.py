import re

# ------------- Fix main.cpp (Shadow Map Camera) -------------
with open('src/main.cpp', 'r') as f:
    main_text = f.read()

# Replace the shadow camera setup
old_light_pos = 'float lightPos[3] = {0.0f, 20.0f, -8.0f};'
new_light_pos = 'float lightPos[3] = {0.0f, 30.0f, 0.0f}; // Moved high up center for orthogonal shadows'
main_text = main_text.replace(old_light_pos, new_light_pos)

old_proj = 'gluPerspective(60.0f, (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT, 2.0f, 80.0f);'
new_proj = 'glOrtho(-19.0f, 19.0f, -25.0f, 25.0f, 1.0f, 60.0f); // Orthogonal projection for straight-down even shadows'
main_text = main_text.replace(old_proj, new_proj)

old_lookat = '''gluLookAt(lightPos[0], lightPos[1], lightPos[2],
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);'''
new_lookat = '''gluLookAt(lightPos[0], lightPos[1], lightPos[2],
              0.0f, 0.0f, 0.0f,
              0.0f, 0.0f, -1.0f); // Look straight down, Up is -Z'''
main_text = main_text.replace(old_lookat, new_lookat)

with open('src/main.cpp', 'w') as f:
    f.write(main_text)


# ------------- Fix lighting.cpp (Turn Keylight into Directional) -------------
with open('src/render/lighting.cpp', 'r') as f:
    light_text = f.read()

# Remove spotlight behavior from GL_LIGHT0 in initLighting
light_text = re.sub(r'glLightf\(GL_LIGHT0,\s*GL_SPOT_CUTOFF,\s*[\d\.]+f\);', 'glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);', light_text)
light_text = re.sub(r'glLightf\(GL_LIGHT0,\s*GL_SPOT_EXPONENT,\s*[\d\.]+f\);', 'glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0f);', light_text)

# Change GL_LIGHT0 position from positional (w=1) to directional (w=0) shining straight down
old_key_pos = 'GLfloat keyPos[] = {0.0f, 20.0f, -8.0f, 1.0f};'
new_key_pos = 'GLfloat keyPos[] = {0.0f, 1.0f, 0.0f, 0.0f}; // Directional view straight down'
light_text = light_text.replace(old_key_pos, new_key_pos)

old_key_dir = 'GLfloat keyDir[] = {0.0f, -1.0f, -0.25f};'
new_key_dir = 'GLfloat keyDir[] = {0.0f, -1.0f, 0.0f};'
light_text = light_text.replace(old_key_dir, new_key_dir)

with open('src/render/lighting.cpp', 'w') as f:
    f.write(light_text)

print("Shadows fixed!")
