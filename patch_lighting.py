import re

with open('src/render/lighting.cpp', 'r') as f:
    text = f.read()

replacement = '''
void positionLights() {
    if (gCeilingLightsEnabled) {
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT2);
        
        GLfloat keyAmbient[] = {0.05f, 0.04f, 0.04f, 1.0f};
        GLfloat keyDiffuse[] = {0.96f, 0.88f, 0.72f, 1.0f};
        GLfloat keySpecular[] = {0.80f, 0.76f, 0.64f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, keyAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, keyDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, keySpecular);

        GLfloat fillAmbient[] = {0.04f, 0.04f, 0.04f, 1.0f};
        GLfloat fillDiffuse[] = {0.46f, 0.43f, 0.38f, 1.0f};
        GLfloat fillSpecular[] = {0.20f, 0.20f, 0.18f, 1.0f};
        glLightfv(GL_LIGHT2, GL_AMBIENT, fillAmbient);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, fillDiffuse);
        glLightfv(GL_LIGHT2, GL_SPECULAR, fillSpecular);
    } else {
        glDisable(GL_LIGHT0);
        glDisable(GL_LIGHT2);
        
        GLfloat zero[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, zero);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, zero);
        glLightfv(GL_LIGHT0, GL_SPECULAR, zero);
        
        glLightfv(GL_LIGHT2, GL_AMBIENT, zero);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, zero);
        glLightfv(GL_LIGHT2, GL_SPECULAR, zero);
    }

'''

text = re.sub(r'void positionLights\(\) \{\s*if \(gCeilingLightsEnabled\) \{\s*glEnable\(GL_LIGHT0\);\s*glEnable\(GL_LIGHT2\);\s*\} else \{\s*glDisable\(GL_LIGHT0\);\s*glDisable\(GL_LIGHT2\);\s*\}', replacement.strip(), text)

with open('src/render/lighting.cpp', 'w') as f:
    f.write(text)

