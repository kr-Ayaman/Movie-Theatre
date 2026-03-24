#include "scene/stage.h"

#include "render/lighting.h"
#include "render/primitives.h"

#include <GL/glut.h>

#include <cmath>

namespace {
const float kPi = 3.14159265f;

void drawScreenContent() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 9.2f, -19.04f);

    glBegin(GL_QUADS);
    glColor3f(0.03f, 0.06f, 0.13f);
    glVertex3f(-11.0f, -4.7f, 0.0f);
    glColor3f(0.07f, 0.16f, 0.32f);
    glVertex3f(11.0f, -4.7f, 0.0f);
    glColor3f(0.16f, 0.28f, 0.55f);
    glVertex3f(11.0f, 4.7f, 0.0f);
    glColor3f(0.07f, 0.12f, 0.25f);
    glVertex3f(-11.0f, 4.7f, 0.0f);
    glEnd();

    drawSoftCircle(-4.5f, 1.6f, 3.3f, 0.38f, 0.20f, 0.74f, 0.42f);
    drawSoftCircle(2.8f, 0.8f, 3.8f, 0.10f, 0.58f, 0.86f, 0.40f);
    drawSoftCircle(1.3f, -1.7f, 2.9f, 0.66f, 0.14f, 0.70f, 0.36f);
    drawSoftCircle(-1.4f, 2.8f, 2.3f, 0.20f, 0.80f, 0.98f, 0.30f);

    glColor4f(0.03f, 0.03f, 0.05f, 0.94f);
    glBegin(GL_POLYGON);
    glVertex3f(-0.4f, -0.8f, 0.01f);
    glVertex3f(0.0f, 0.2f, 0.01f);
    glVertex3f(0.7f, 1.3f, 0.01f);
    glVertex3f(1.4f, 1.8f, 0.01f);
    glVertex3f(2.0f, 1.2f, 0.01f);
    glVertex3f(1.7f, 0.3f, 0.01f);
    glVertex3f(1.0f, -0.5f, 0.01f);
    glVertex3f(0.2f, -1.0f, 0.01f);
    glEnd();

    glColor4f(0.03f, 0.03f, 0.05f, 0.94f);
    glBegin(GL_POLYGON);
    glVertex3f(-2.8f, -0.9f, 0.01f);
    glVertex3f(-2.2f, 0.5f, 0.01f);
    glVertex3f(-1.7f, 1.9f, 0.01f);
    glVertex3f(-1.1f, 1.4f, 0.01f);
    glVertex3f(-1.2f, 0.3f, 0.01f);
    glVertex3f(-1.7f, -0.7f, 0.01f);
    glEnd();

    glColor4f(0.50f, 0.66f, 0.88f, 0.18f);
    glBegin(GL_LINES);
    for (int i = 0; i < 20; ++i) {
        float y = -4.4f + i * 0.45f;
        glVertex3f(-10.8f, y, 0.02f);
        glVertex3f(10.8f, y + 0.08f, 0.02f);
    }
    glEnd();

    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawCurtain(bool leftSide) {
    float side = leftSide ? -1.0f : 1.0f;
    float startX = leftSide ? -13.3f : 13.3f;

    for (int fold = 0; fold < 9; ++fold) {
        float offset = fold * 0.62f * side;
        float depthWave = std::sin(fold * 0.9f) * 0.35f;
        float tint = 0.22f + (depthWave + 0.35f) * 0.05f;
        setMaterial(0.07f, tint, 0.52f, 45.0f, 0.30f);
        drawBlock(startX + offset, 8.5f, -10.8f + depthWave, 0.55f, 14.5f, 1.25f);
    }
}

void drawBeanBag(float x, float z) {
    setMaterial(0.18f, 0.34f, 0.58f, 28.0f, 0.24f);
    glPushMatrix();
    glTranslatef(x, 1.35f, z);
    glScalef(1.4f, 0.56f, 1.1f);
    glutSolidSphere(1.0, 26, 20);
    glPopMatrix();
}

}  // namespace

void drawStageAndScreen() {
    setMaterial(0.36f, 0.24f, 0.17f, 30.0f, 0.15f);
    drawBlock(0.0f, 1.5f, -14.0f, 28.0f, 2.0f, 12.0f);
    drawBlock(0.0f, 0.6f, -7.4f, 23.0f, 0.6f, 2.6f);

    setMaterial(0.10f, 0.11f, 0.13f, 18.0f, 0.20f);
    drawBlock(-14.2f, 8.6f, -11.6f, 1.2f, 15.2f, 1.5f);
    drawBlock(14.2f, 8.6f, -11.6f, 1.2f, 15.2f, 1.5f);
    drawBlock(0.0f, 15.5f, -11.6f, 30.0f, 1.3f, 1.5f);

    setMaterial(0.08f, 0.09f, 0.10f, 15.0f, 0.16f);
    drawBlock(0.0f, 9.2f, -19.6f, 24.7f, 11.6f, 0.90f);

    setMaterial(0.14f, 0.16f, 0.21f, 22.0f, 0.24f, 0.06f);
    drawBlock(0.0f, 9.2f, -19.2f, 22.6f, 9.8f, 0.25f);
    drawScreenContent();

    drawCurtain(true);
    drawCurtain(false);

    drawBeanBag(-1.8f, -8.2f);
    drawBeanBag(1.8f, -8.3f);
}
