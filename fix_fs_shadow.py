import re
with open("src/render/shader.cpp", "r") as f:
    text = f.read()

# GL_LIGHT1 is the screen light. We need to allow it to cast shadows or at least light the back wall. 
# Right now it has zero spot attenuation logic issues due to spot direction.
# If we want the screen to throw shadows on the back wall, we should allow shadow mask on all lights.

# Actually, the screen (GL_LIGHT1) does NOT cast shadows in your code! 
# We explicitly set it up earlier: float sf = (lightIndex == 0) ? shadowFactor : 0.0;
# But because of normal maps and spotlight cone, the screen light might not reach the back wall.

# Let's fix the spot light of the screen so it reaches the back.
with open("src/render/lighting.cpp", "r") as f:
    lt = f.read()

# Increase screen light brightness and spread
lt = lt.replace('GLfloat screenDiffuse[] = {0.26f, 0.44f, 0.70f, 1.0f};', 'GLfloat screenDiffuse[] = {0.86f, 1.24f, 1.70f, 1.0f};')
lt = lt.replace('glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 8.0f);', 'glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1.0f);')
lt = lt.replace('glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 70.0f);', 'glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 90.0f);')

with open("src/render/lighting.cpp", "w") as f:
    f.write(lt)

