import re

with open('src/render/lighting.cpp', 'r') as f:
    text = f.read()

# Replace in initLighting
text = text.replace('GLfloat keyDiffuse[] = {0.95f, 0.90f, 0.82f, 1.0f};', 'GLfloat keyDiffuse[] = {1.30f, 1.25f, 1.15f, 1.0f};')
text = text.replace('GLfloat keySpecular[] = {0.95f, 0.92f, 0.87f, 1.0f};', 'GLfloat keySpecular[] = {1.0f, 0.98f, 0.95f, 1.0f};')
text = text.replace('GLfloat keyAmbient[] = {0.07f, 0.06f, 0.05f, 1.0f};', 'GLfloat keyAmbient[] = {0.12f, 0.11f, 0.10f, 1.0f};')

text = text.replace('GLfloat fillDiffuse[] = {0.46f, 0.43f, 0.38f, 1.0f};', 'GLfloat fillDiffuse[] = {0.70f, 0.68f, 0.62f, 1.0f};')
text = text.replace('GLfloat fillAmbient[] = {0.04f, 0.04f, 0.04f, 1.0f};', 'GLfloat fillAmbient[] = {0.08f, 0.08f, 0.08f, 1.0f};')

# Replace in positionLights
text = text.replace('GLfloat keyDiffuse[] = {0.96f, 0.88f, 0.72f, 1.0f};', 'GLfloat keyDiffuse[] = {1.30f, 1.25f, 1.15f, 1.0f};')
text = text.replace('GLfloat keySpecular[] = {0.80f, 0.76f, 0.64f, 1.0f};', 'GLfloat keySpecular[] = {1.0f, 0.98f, 0.95f, 1.0f};')
text = text.replace('GLfloat keyAmbient[] = {0.05f, 0.04f, 0.04f, 1.0f};', 'GLfloat keyAmbient[] = {0.12f, 0.11f, 0.10f, 1.0f};')

with open('src/render/lighting.cpp', 'w') as f:
    f.write(text)
