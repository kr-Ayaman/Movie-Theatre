#include "render/lighting.h"

#include <GL/glut.h>

void setMaterial(float r, float g, float b, float shininess, float specularStrength, float emissionStrength) {
    GLfloat ambient[] = {r * 0.35f, g * 0.35f, b * 0.35f, 1.0f};
    GLfloat diffuse[] = {r, g, b, 1.0f};
    GLfloat specular[] = {specularStrength, specularStrength, specularStrength, 1.0f};
    GLfloat emission[] = {r * emissionStrength, g * emissionStrength, b * emissionStrength, 1.0f};

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

    GLfloat globalAmbient[] = {0.14f, 0.14f, 0.17f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    GLfloat keyAmbient[] = {0.07f, 0.06f, 0.05f, 1.0f};
    GLfloat keyDiffuse[] = {0.95f, 0.90f, 0.82f, 1.0f};
    GLfloat keySpecular[] = {0.95f, 0.92f, 0.87f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, keyAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, keyDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, keySpecular);
    glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 36.0f);
    glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 22.0f);

    GLfloat screenAmbient[] = {0.01f, 0.02f, 0.04f, 1.0f};
    GLfloat screenDiffuse[] = {0.26f, 0.44f, 0.70f, 1.0f};
    GLfloat screenSpecular[] = {0.20f, 0.30f, 0.45f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, screenAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, screenDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, screenSpecular);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 70.0f);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 8.0f);

    GLfloat fillAmbient[] = {0.04f, 0.04f, 0.04f, 1.0f};
    GLfloat fillDiffuse[] = {0.46f, 0.43f, 0.38f, 1.0f};
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
    GLfloat keyPos[] = {0.0f, 20.0f, -8.0f, 1.0f};
    GLfloat keyDir[] = {0.0f, -1.0f, -0.25f};
    glLightfv(GL_LIGHT0, GL_POSITION, keyPos);
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, keyDir);

    GLfloat screenPos[] = {0.0f, 10.5f, -20.0f, 1.0f};
    GLfloat screenDir[] = {0.0f, -0.2f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, screenPos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, screenDir);

    GLfloat fillPos[] = {0.0f, 20.0f, 14.0f, 1.0f};
    glLightfv(GL_LIGHT2, GL_POSITION, fillPos);

    GLfloat viewFillPos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    GLfloat viewFillDir[] = {0.0f, 0.0f, -1.0f};
    glLightfv(GL_LIGHT3, GL_POSITION, viewFillPos);
    glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, viewFillDir);
}
