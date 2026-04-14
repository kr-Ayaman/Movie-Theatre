#include "scene/room.h"

#include "render/lighting.h"
#include "render/primitives.h"
#include "render/shader.h"

#include <GL/glut.h>

namespace {

bool gCeilingLightsVisible = true;

void drawSimpleWalls() {
    setSceneShaderEffect(kSceneShaderEffectBrick);
    setMaterial(0.73f, 0.74f, 0.76f, 9.0f, 0.04f);

    // Four enclosing walls: left, right, back, and front.
    drawBlock(-17.0f, 11.0f, 0.0f, 0.6f, 22.0f, 48.0f);
    drawBlock(17.0f, 11.0f, 0.0f, 0.6f, 22.0f, 48.0f);
    drawBlock(0.0f, 11.0f, -22.0f, 34.0f, 22.0f, 0.6f);
    drawBlock(0.0f, 11.0f, 22.0f, 34.0f, 22.0f, 0.6f);

    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawCeilingSurface() {
    setSceneShaderEffect(kSceneShaderEffectCeiling);
    setMaterial(0.58f, 0.60f, 0.64f, 7.0f, 0.04f);
    drawBlock(0.0f, 21.5f, 0.0f, 34.0f, 0.5f, 48.0f);
    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawCeilingLights() {
    const int cols = 11;
    const int rows = 7;
    const float xMin = -15.2f;
    const float xMax = 15.2f;
    const float zMin = -20.6f;
    const float zMax = 20.6f;

    for (int row = 0; row < rows; ++row) {
        float z = zMin + (zMax - zMin) * static_cast<float>(row) / static_cast<float>(rows - 1);
        for (int col = 0; col < cols; ++col) {
            float x = xMin + (xMax - xMin) * static_cast<float>(col) / static_cast<float>(cols - 1);

            if (gCeilingLightsVisible) {
                setMaterial(1.0f, 0.96f, 0.88f, 75.0f, 0.85f, 0.40f);
            } else {
                setMaterial(0.17f, 0.17f, 0.18f, 6.0f, 0.02f, 0.0f);
            }

            glPushMatrix();
            glTranslatef(x, 20.72f, z);
            glutSolidSphere(0.14, 14, 10);
            glPopMatrix();
        }
    }
}

}  // namespace

void drawRoom() {
    setSceneShaderEffect(kSceneShaderEffectDefault);

    setMaterial(0.24f, 0.17f, 0.12f, 25.0f, 0.14f);
    drawBlock(0.0f, -0.2f, 0.0f, 34.0f, 0.6f, 48.0f);

    drawSimpleWalls();
    drawCeilingSurface();

    for (int plank = 0; plank < 18; ++plank) {
        float z = -20.0f + plank * 2.3f;
        float n = noise2D(plank, 77);
        setMaterial(0.24f + n * 0.08f, 0.17f + n * 0.05f, 0.12f + n * 0.05f, 20.0f, 0.10f);
        drawBlock(0.0f, 0.02f, z, 33.0f, 0.06f, 1.9f);
    }

    drawCeilingLights();
}

void toggleCeilingLights() {
    gCeilingLightsVisible = !gCeilingLightsVisible;
}

bool areCeilingLightsVisible() {
    return gCeilingLightsVisible;
}
