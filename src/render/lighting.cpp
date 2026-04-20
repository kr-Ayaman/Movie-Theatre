#include "render/lighting.h"

#include <GL/glut.h>

namespace {
bool gCeilingLightsEnabled = true;
}

void setMaterial(float r, float g, float b, float shininess, float specularStrength, float emissionStrength) {
    GLfloat ambient[] = {r * 0.35f, g * 0.35f, b * 0.35f, 1.0f};
    GLfloat diffuse[] = {r, g, b, 1.0f};
    GLfloat specular[] = {specularStrength, specularStrength, specularStrength, 1.0f};
    GLfloat emission[] = {r * emissionStrength, g * emissionStrength, b * emissionStrength, 1.0f};

    // Keep material-driven objects on the lighting branch in the shader.
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
}

void initLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
    glEnable(GL_LIGHT3);

    GLfloat globalAmbient[] = {0.35f, 0.35f, 0.40f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    GLfloat keyAmbient[] = {0.12f, 0.11f, 0.10f, 1.0f};
    GLfloat keyDiffuse[] = {1.30f, 1.25f, 1.15f, 1.0f};
    GLfloat keySpecular[] = {1.0f, 0.98f, 0.95f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, keyAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, keyDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, keySpecular);
    // glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);
    // glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 0.0f);

    GLfloat screenAmbient[] = {0.01f, 0.02f, 0.04f, 1.0f};
    GLfloat screenDiffuse[] = {0.86f, 1.24f, 1.70f, 1.0f};
    GLfloat screenSpecular[] = {0.20f, 0.30f, 0.45f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, screenAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, screenDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, screenSpecular);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 90.0f);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 1.0f);

    GLfloat fillAmbient[] = {0.08f, 0.08f, 0.08f, 1.0f};
    GLfloat fillDiffuse[] = {0.70f, 0.68f, 0.62f, 1.0f};
    GLfloat fillSpecular[] = {0.20f, 0.20f, 0.18f, 1.0f};
    glLightfv(GL_LIGHT2, GL_AMBIENT, fillAmbient);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, fillDiffuse);
    glLightfv(GL_LIGHT2, GL_SPECULAR, fillSpecular);

    // Subtle camera-aligned fill keeps seat backs readable from rear angles.
    GLfloat viewFillAmbient[] = {0.02f, 0.02f, 0.02f, 1.0f};
    GLfloat viewFillDiffuse[] = {0.20f, 0.19f, 0.18f, 1.0f};
    GLfloat viewFillSpecular[] = {0.08f, 0.08f, 0.08f, 1.0f};
    glLightfv(GL_LIGHT3, GL_AMBIENT, viewFillAmbient);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, viewFillDiffuse);
    glLightfv(GL_LIGHT3, GL_SPECULAR, viewFillSpecular);
    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 95.0f);
    glLightf(GL_LIGHT3, GL_SPOT_EXPONENT, 4.0f);
}

void positionLights() {
    if (gCeilingLightsEnabled) {
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT2);
        
        GLfloat keyAmbient[] = {0.12f, 0.11f, 0.10f, 1.0f};
        GLfloat keyDiffuse[] = {1.30f, 1.25f, 1.15f, 1.0f};
        GLfloat keySpecular[] = {1.0f, 0.98f, 0.95f, 1.0f};
        glLightfv(GL_LIGHT0, GL_AMBIENT, keyAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, keyDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, keySpecular);

        GLfloat fillAmbient[] = {0.08f, 0.08f, 0.08f, 1.0f};
        GLfloat fillDiffuse[] = {0.70f, 0.68f, 0.62f, 1.0f};
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

    GLfloat keyPos[] = {0.0f, 1.0f, 0.0f, 0.0f}; // Directional light from above
    // No spot direction for directional light
    glLightfv(GL_LIGHT0, GL_POSITION, keyPos);
    //

    GLfloat screenPos[] = {0.0f, 10.5f, -20.0f, 1.0f};
    GLfloat screenDir[] = {0.0f, -0.2f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, screenPos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, screenDir);

    GLfloat fillPos[] = {0.0f, 20.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, fillPos);

    GLfloat viewFillPos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat viewFillDir[] = {0.0f, 0.0f, -1.0f};
    glLightfv(GL_LIGHT3, GL_POSITION, viewFillPos);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, viewFillDir);
}

void setCeilingLightsEnabled(bool enabled) {
    gCeilingLightsEnabled = enabled;
}

bool areCeilingLightsEnabled() {
    return gCeilingLightsEnabled;
}
