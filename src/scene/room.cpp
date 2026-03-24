#include "scene/room.h"

#include "render/lighting.h"
#include "render/primitives.h"

#include <GL/glut.h>

namespace {

void drawWallTiles() {
    for (int row = 0; row < 10; ++row) {
        float y = 2.1f + row * 2.0f;
        for (int col = 0; col < 18; ++col) {
            float z = -19.5f + col * 2.2f;

            float leftNoise = noise2D(row + 30, col + 80);
            float leftR = 0.52f + leftNoise * 0.19f;
            float leftG = 0.42f + leftNoise * 0.13f;
            float leftB = 0.35f + leftNoise * 0.10f;
            setMaterial(leftR, leftG, leftB, 10.0f, 0.06f);
            drawBlock(-16.7f, y, z, 0.12f, 1.9f, 2.05f);

            float rightNoise = noise2D(row + 140, col + 12);
            float rightR = 0.50f + rightNoise * 0.20f;
            float rightG = 0.41f + rightNoise * 0.14f;
            float rightB = 0.34f + rightNoise * 0.10f;
            setMaterial(rightR, rightG, rightB, 10.0f, 0.06f);
            drawBlock(16.7f, y, z, 0.12f, 1.9f, 2.05f);
        }
    }

    for (int row = 0; row < 10; ++row) {
        float y = 2.1f + row * 2.0f;
        for (int col = 0; col < 15; ++col) {
            float x = -14.3f + col * 2.05f;
            float n = noise2D(row + 200, col + 300);
            float r = 0.48f + n * 0.22f;
            float g = 0.39f + n * 0.14f;
            float b = 0.32f + n * 0.12f;
            setMaterial(r, g, b, 10.0f, 0.05f);
            drawBlock(x, y, -21.7f, 1.9f, 1.9f, 0.12f);
        }
    }
}

void drawCeilingGrid() {
    glDisable(GL_LIGHTING);
    glColor4f(0.78f, 0.80f, 0.84f, 0.40f);
    glBegin(GL_LINES);
    for (float x = -16.0f; x <= 16.0f; x += 2.0f) {
        glVertex3f(x, 21.24f, -22.0f);
        glVertex3f(x, 21.24f, 22.0f);
    }
    for (float z = -22.0f; z <= 22.0f; z += 2.0f) {
        glVertex3f(-16.0f, 21.24f, z);
        glVertex3f(16.0f, 21.24f, z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawCeilingLights() {
    for (int i = 0; i < 13; ++i) {
        float x = -14.0f + i * 2.3f;
        for (int band = 0; band < 2; ++band) {
            float z = (band == 0) ? -17.0f : 10.5f;
            setMaterial(1.0f, 0.94f, 0.82f, 70.0f, 0.80f, 0.35f);
            glPushMatrix();
            glTranslatef(x, 20.7f, z);
            glutSolidSphere(0.18, 18, 14);
            glPopMatrix();
        }
    }
}

}  // namespace

void drawRoom() {
    setMaterial(0.24f, 0.17f, 0.12f, 25.0f, 0.14f);
    drawBlock(0.0f, -0.2f, 0.0f, 34.0f, 0.6f, 48.0f);

    setMaterial(0.19f, 0.15f, 0.13f, 7.0f, 0.04f);
    drawBlock(-17.0f, 11.0f, 0.0f, 0.6f, 22.0f, 48.0f);
    drawBlock(17.0f, 11.0f, 0.0f, 0.6f, 22.0f, 48.0f);
    drawBlock(0.0f, 11.0f, -22.0f, 34.0f, 22.0f, 0.6f);

    setMaterial(0.08f, 0.09f, 0.11f, 8.0f, 0.05f);
    drawBlock(0.0f, 21.5f, 0.0f, 34.0f, 0.5f, 48.0f);

    drawWallTiles();

    for (int plank = 0; plank < 18; ++plank) {
        float z = -20.0f + plank * 2.3f;
        float n = noise2D(plank, 77);
        setMaterial(0.24f + n * 0.08f, 0.17f + n * 0.05f, 0.12f + n * 0.05f, 20.0f, 0.10f);
        drawBlock(0.0f, 0.02f, z, 33.0f, 0.06f, 1.9f);
    }

    drawCeilingGrid();
    drawCeilingLights();
}
