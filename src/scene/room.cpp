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

void drawSpeakerDetailed(float width, float height, float depth, bool faceX) {
    // Main speaker casing
    setMaterial(0.05f, 0.05f, 0.06f, 20.0f, 0.15f);
    drawBlock(0.0f, 0.0f, 0.0f, width, height, depth);
    
    // Front grill/fabric (darker, unreflective)
    setMaterial(0.02f, 0.02f, 0.02f, 5.0f, 0.02f);
    if (faceX) {
        drawBlock(width * 0.46f, 0.0f, 0.0f, 0.12f, height * 0.88f, depth * 0.88f);
    } else {
        drawBlock(0.0f, 0.0f, -depth * 0.46f, width * 0.88f, height * 0.88f, 0.12f);
    }
}

void drawSignLetterE(float x, float y, float z, bool lit) {
    setMaterial(0.0f, lit ? 0.98f : 0.25f, 0.0f, 10.0f, 0.25f, lit ? 1.0f : 0.12f);
    drawBlock(x, y, z, 0.08f, 0.60f, 0.10f);
    drawBlock(x, y + 0.22f, z + 0.15f, 0.08f, 0.10f, 0.60f);
    drawBlock(x, y, z + 0.15f, 0.08f, 0.10f, 0.60f);
    drawBlock(x, y - 0.22f, z + 0.15f, 0.08f, 0.10f, 0.60f);
}

void drawSignLetterI(float x, float y, float z, bool lit) {
    setMaterial(0.0f, lit ? 0.98f : 0.25f, 0.0f, 10.0f, 0.25f, lit ? 1.0f : 0.12f);
    drawBlock(x, y, z, 0.08f, 0.60f, 0.10f);
}

void drawSignLetterT(float x, float y, float z, bool lit) {
    setMaterial(0.0f, lit ? 0.98f : 0.25f, 0.0f, 10.0f, 0.25f, lit ? 1.0f : 0.12f);
    drawBlock(x, y + 0.25f, z, 0.10f, 0.10f, 0.60f);
    drawBlock(x, y - 0.05f, z, 0.08f, 0.50f, 0.10f);
}

void drawSignLetterX(float x, float y, float z, bool lit) {
    setMaterial(0.0f, lit ? 0.98f : 0.25f, 0.0f, 10.0f, 0.25f, lit ? 1.0f : 0.12f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.06f, 0.38f, 0.10f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-45.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.06f, 0.38f, 0.10f);
    glPopMatrix();
}

void drawExitDoor(bool lit) {
    float doorX = 16.45f;
    float doorY = 2.60f; // bottom sits on the ground
    float doorZ = -4.20f; // move the door further forward ahead of the front row

    setSceneShaderEffect(kSceneShaderEffectDefault);
    setMaterial(0.08f, 0.08f, 0.10f, 12.0f, 0.06f, 0.05f);
    drawBlock(doorX, doorY, doorZ, 0.14f, 5.20f, 3.40f);

    setMaterial(0.04f, 0.05f, 0.06f, 8.0f, 0.04f, lit ? 0.40f : 0.08f);
    drawBlock(doorX - 0.03f, doorY, doorZ, 0.06f, 4.80f, 2.80f);

    if (lit) {
        setMaterial(0.0f, 0.80f, 0.0f, 10.0f, 0.30f, 1.0f);
    } else {
        setMaterial(0.0f, 0.25f, 0.0f, 10.0f, 0.10f, 0.15f);
    }
    drawBlock(doorX + 0.08f, doorY, doorZ, 0.03f, 5.20f, 3.20f);

    float signX = doorX + 0.14f;
    float signY = doorY + 3.70f;
    float signZ = doorZ;
    setMaterial(0.02f, 0.02f, 0.02f, 4.0f, 0.03f, 0.0f);
    drawBlock(signX - 0.02f, signY, signZ, 0.18f, 0.80f, 4.20f);

    float letterX = signX + 0.06f;
    float letterTopY = signY;
    float letterZ0 = signZ - 1.70f;
    float letterGap = 1.05f;

    drawSignLetterE(letterX, letterTopY, letterZ0, lit);
    drawSignLetterX(letterX, letterTopY, letterZ0 + letterGap, lit);
    drawSignLetterI(letterX, letterTopY, letterZ0 + 2.05f, lit);
    drawSignLetterT(letterX, letterTopY, letterZ0 + 3.05f, lit);
}

void drawWallSpeakers() {
    setSceneShaderEffect(kSceneShaderEffectDefault);

    // Side walls: 6 speakers on each side
    for (float z = -12.0f; z <= 18.0f; z += 6.0f) {
        // Calculate a dynamic height that follows the slope of the stadium seating
        float yHeight = 8.5f;
        if (z > 0.0f) {
            yHeight += (z / 1.55f) * 0.78f; // Follow the rise/run of the seats
        }

        // Left wall speaker
        glPushMatrix();
        glTranslatef(-16.6f, yHeight, z);
        glRotatef(12.0f, 0.0f, 1.0f, 0.0f); // turn inwards slightly towards screen
        glRotatef(-15.0f, 0.0f, 0.0f, 1.0f); // tilt down towards audience
        drawSpeakerDetailed(0.5f, 1.6f, 1.0f, true);
        glPopMatrix();

        // Right wall speaker
        glPushMatrix();
        glTranslatef(16.6f, yHeight, z);
        glRotatef(-12.0f, 0.0f, 1.0f, 0.0f); // turn inwards slightly towards screen
        glRotatef(15.0f, 0.0f, 0.0f, 1.0f); // tilt down towards audience
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f); // flip so face points into room
        drawSpeakerDetailed(0.5f, 1.6f, 1.0f, true);
        glPopMatrix();
    }

    // Back wall: 3 speakers
    // The top row of seats is high up, so the back wall speakers need to be proportionally higher
    float backWallHeight = 8.5f + (21.6f / 1.55f) * 0.78f;
    float backX[] = {-8.0f, 0.0f, 8.0f};
    for (float x : backX) {
        glPushMatrix();
        glTranslatef(x, backWallHeight, 21.6f);
        glRotatef(20.0f, 1.0f, 0.0f, 0.0f); // tilt down more heavily into room
        drawSpeakerDetailed(1.0f, 1.6f, 0.5f, false);
        glPopMatrix();
    }
}

}  // namespace

void drawRoom() {
    setSceneShaderEffect(kSceneShaderEffectDefault);

    setMaterial(0.24f, 0.17f, 0.12f, 25.0f, 0.14f);
    drawBlock(0.0f, -0.2f, 0.0f, 34.0f, 0.6f, 48.0f);

    drawSimpleWalls();
    drawExitDoor(gCeilingLightsVisible);
    drawCeilingSurface();
    drawWallSpeakers();

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
