#include "render/primitives.h"

#include <GL/glut.h>

#include <cmath>

namespace {
const float kPi = 3.14159265f;
}

float noise2D(int x, int y) {
    float value = std::sin(x * 12.9898f + y * 78.233f) * 43758.5453f;
    return value - std::floor(value);
}

void drawBlock(float x, float y, float z, float width, float height, float depth) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width, height, depth);
    glutSolidCube(1.0f);
    glPopMatrix();
}

void drawSoftCircle(float cx, float cy, float radius, float r, float g, float b, float alpha) {
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(r, g, b, alpha);
    glVertex3f(cx, cy, 0.0f);
    glColor4f(r, g, b, 0.0f);
    for (int i = 0; i <= 36; ++i) {
        float angle = 2.0f * kPi * static_cast<float>(i) / 36.0f;
        glVertex3f(cx + std::cos(angle) * radius, cy + std::sin(angle) * radius, 0.0f);
    }
    glEnd();
}
