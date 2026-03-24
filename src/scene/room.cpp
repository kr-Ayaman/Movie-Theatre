#include "scene/room.h"

#include "render/lighting.h"
#include "render/primitives.h"

#include <GL/glut.h>

namespace {

void drawWallTiles() {
    const int courses = 17;
    const float courseHeight = 1.18f;
    const float brickHeight = 1.05f;
    const float firstCourseY = 1.3f;

    const float sideStart = -21.0f;
    const float sideEnd = 21.0f;
    const float backStart = -15.5f;
    const float backEnd = 15.5f;

    const float sideInset = 16.66f;
    const float backInset = -21.66f;

    for (int row = 0; row < courses; ++row) {
        float y = firstCourseY + row * courseHeight;
        float offset = (row % 2 == 0) ? 0.0f : 0.75f;

        float runSide = sideStart + offset;
        int segment = 0;
        while (runSide < sideEnd) {
            float selector = noise2D(row + 22, segment + 31);
            float brickLen = 1.1f;
            if (selector > 0.70f) {
                brickLen = 2.2f;
            } else if (selector > 0.35f) {
                brickLen = 1.6f;
            }

            if (runSide + brickLen > sideEnd) {
                brickLen = sideEnd - runSide;
            }
            if (brickLen < 0.3f) {
                break;
            }

            float toneA = noise2D(row + 100, segment + 5);
            float toneB = noise2D(row + 170, segment + 55);
            float r = 0.48f + toneA * 0.22f;
            float g = 0.39f + toneB * 0.16f;
            float b = 0.31f + toneA * 0.13f;
            setMaterial(r, g, b, 10.0f, 0.05f);

            float zCenter = runSide + brickLen * 0.5f;
            drawBlock(-sideInset, y, zCenter, 0.13f, brickHeight, brickLen - 0.08f);
            drawBlock(sideInset, y, zCenter, 0.13f, brickHeight, brickLen - 0.08f);

            runSide += brickLen;
            segment++;
        }

        float runBack = backStart + offset;
        int backSegment = 0;
        while (runBack < backEnd) {
            float selector = noise2D(row + 260, backSegment + 19);
            float brickLen = 1.15f;
            if (selector > 0.68f) {
                brickLen = 2.05f;
            } else if (selector > 0.30f) {
                brickLen = 1.45f;
            }

            if (runBack + brickLen > backEnd) {
                brickLen = backEnd - runBack;
            }
            if (brickLen < 0.3f) {
                break;
            }

            float toneA = noise2D(row + 330, backSegment + 67);
            float toneB = noise2D(row + 410, backSegment + 13);
            float r = 0.47f + toneA * 0.24f;
            float g = 0.38f + toneB * 0.16f;
            float b = 0.30f + toneA * 0.14f;
            setMaterial(r, g, b, 10.0f, 0.05f);

            float xCenter = runBack + brickLen * 0.5f;
            drawBlock(xCenter, y, backInset, brickLen - 0.08f, brickHeight, 0.13f);

            runBack += brickLen;
            backSegment++;
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
