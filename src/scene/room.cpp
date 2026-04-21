#include "scene/room.h"

#include "render/lighting.h"
#include "render/primitives.h"
#include "render/shader.h"

#include <GL/glut.h>

namespace {

bool gCeilingLightsVisible = true;
bool gInShadowPass = false;

constexpr float kWallThickness = 0.6f;
constexpr float kRoomWidth = 36.0f;
constexpr float kRoomDepth = 48.0f;
constexpr float kSideWallX = 18.0f;
constexpr float kFrontBackWallZ = 22.0f;
constexpr float kWallFixtureX = 17.6f;
constexpr float kFloorPlankWidth = 35.0f;

void drawSimpleWalls() {
    // Skip walls in shadow pass: wall blocks self-shadow (wall top occludes
    // the wall body from above), causing incorrect shadow on wall surfaces.
    // Speakers (drawn separately) still cast shadows on both walls.
    if (gInShadowPass) return;
    setSceneShaderEffect(kSceneShaderEffectBrick);
    setMaterial(0.73f, 0.74f, 0.76f, 9.0f, 0.04f);

    // Four enclosing walls: left, right, back, and front.
    drawBlock(-kSideWallX, 11.0f, 0.0f, kWallThickness, 22.0f, kRoomDepth);
    drawBlock(kSideWallX, 11.0f, 0.0f, kWallThickness, 22.0f, kRoomDepth);
    drawBlock(0.0f, 11.0f, -kFrontBackWallZ, kRoomWidth, 22.0f, kWallThickness);
    drawBlock(0.0f, 11.0f, kFrontBackWallZ, kRoomWidth, 22.0f, kWallThickness);

    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawCeilingSurface() {
    // Skip ceiling in shadow depth pass — it covers the whole scene from above
    // and would make everything uniformly shadowed with no visible contrast.
    if (gInShadowPass) return;
    setSceneShaderEffect(kSceneShaderEffectCeiling);
    setMaterial(0.58f, 0.60f, 0.64f, 7.0f, 0.04f);
    drawBlock(0.0f, 21.5f, 0.0f, kRoomWidth, 0.5f, kRoomDepth);
    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawCeilingLights() {
    // Light bulbs themselves need not cast shadows.
    if (gInShadowPass) return;
    const int cols = 11;
    const int rows = 7;
    const float xMin = -16.0f;
    const float xMax = 16.0f;
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


void setNeonMaterial(bool lit, float r, float g, float b) {
    if (lit) {
        setMaterial(r, g, b, 10.0f, 0.25f, 1.0f);
    } else {
        setMaterial(r * 0.25f, g * 0.25f, b * 0.25f, 10.0f, 0.25f, 0.12f);
    }
}

void drawSignLetterE(float x, float y, float z, bool lit, float r, float g, float b) {
    setNeonMaterial(lit, r, g, b);
    drawBlock(x, y, z, 0.08f, 0.60f, 0.10f);
    drawBlock(x, y + 0.22f, z + 0.15f, 0.08f, 0.10f, 0.30f);
    drawBlock(x, y, z + 0.15f, 0.08f, 0.10f, 0.30f);
    drawBlock(x, y - 0.22f, z + 0.15f, 0.08f, 0.10f, 0.30f);
}

void drawSignLetterI(float x, float y, float z, bool lit, float r, float g, float b) {
    setNeonMaterial(lit, r, g, b);
    drawBlock(x, y, z, 0.08f, 0.60f, 0.10f);
}

void drawSignLetterT(float x, float y, float z, bool lit, float r, float g, float b) {
    setNeonMaterial(lit, r, g, b);
    drawBlock(x, y + 0.25f, z, 0.08f, 0.10f, 0.40f);
    drawBlock(x, y - 0.05f, z, 0.08f, 0.50f, 0.10f);
}

void drawSignLetterX(float x, float y, float z, bool lit, float r, float g, float b) {
    setNeonMaterial(lit, r, g, b);
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(35.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.06f, 0.58f, 0.10f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-35.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.06f, 0.58f, 0.10f);
    glPopMatrix();
}

void drawGate(bool lit, bool isLeftWall) {
    float wallX = isLeftWall ? -kWallFixtureX : kWallFixtureX;
    float doorZ = -5.0f; // Moved near the front stage

    setSceneShaderEffect(kSceneShaderEffectGate);

    // Frame (Pillars and Lintel)
    setMaterial(0.06f, 0.06f, 0.08f, 15.0f, 0.08f, 0.05f);
    // Left Pillar 
    drawBlock(wallX, 3.0f, doorZ - 1.8f, 0.6f, 6.0f, 0.4f);
    // Right Pillar 
    drawBlock(wallX, 3.0f, doorZ + 1.8f, 0.6f, 6.0f, 0.4f);
    // Top Lintel (Skip in shadow pass to avoid large inaccurate shadow on door)
    if (!gInShadowPass) {
        drawBlock(wallX, 5.7f, doorZ, 0.6f, 0.6f, 4.0f);
    }

    // The double doors (recessed slightly into the wall)
    float doorX = isLeftWall ? -(kWallFixtureX + 0.05f) : (kWallFixtureX + 0.05f);
    setMaterial(0.03f, 0.03f, 0.04f, 10.0f, 0.03f, 0.02f); // very dark door surface
    drawBlock(doorX, 2.7f, doorZ, 0.2f, 5.4f, 3.2f);
    
    // Door pushbars (shiny metal handles)
    setSceneShaderEffect(kSceneShaderEffectMetal);
    setMaterial(0.4f, 0.4f, 0.45f, 50.0f, 0.6f, 0.1f);
    float barX = isLeftWall ? -(kWallFixtureX - 0.2f) : (kWallFixtureX - 0.2f);
    drawBlock(barX, 2.7f, doorZ - 0.8f, 0.1f, 0.08f, 1.2f);
    drawBlock(barX, 2.7f, doorZ + 0.8f, 0.1f, 0.08f, 1.2f);
    setSceneShaderEffect(kSceneShaderEffectGate);

    // Center vertical divider for the double doors
    setMaterial(0.01f, 0.01f, 0.01f, 5.0f, 0.0f, 0.0f);
    drawBlock(doorX + (isLeftWall ? 0.05f : -0.05f), 2.7f, doorZ, 0.25f, 5.4f, 0.05f);

    if (!isLeftWall) {
        // Keep the EXIT sign above the right-hand gate only.
        float signX = kWallFixtureX + 0.05f;
        float signY = 6.3f; // mounted above the doorway
        float signZ = doorZ;
        setMaterial(0.01f, 0.01f, 0.01f, 4.0f, 0.02f, 0.0f);
        drawBlock(signX, signY, signZ, 0.1f, 0.5f, 1.8f);

        // Translate and properly scale down the neon text structure
        setSceneShaderEffect(kSceneShaderEffectDefault);
        glPushMatrix();
        glTranslatef(signX - 0.08f, signY, signZ);
        glScalef(0.30f, 0.30f, 0.30f); // Make the letters much smaller!

        float gap = 0.8f;
        float r = 0.98f;
        float g = 0.1f;
        float b = 0.1f;
        float startZ = -1.2f;
        drawSignLetterE(0, 0, startZ, lit, r, g, b);
        drawSignLetterX(0, 0, startZ + gap, lit, r, g, b);
        drawSignLetterI(0, 0, startZ + gap * 2, lit, r, g, b);
        drawSignLetterT(0, 0, startZ + gap * 3, lit, r, g, b);
        glPopMatrix();

        setSceneShaderEffect(kSceneShaderEffectGate);
    }

    setSceneShaderEffect(kSceneShaderEffectDefault);
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
        glTranslatef(-kWallFixtureX, yHeight, z);
        glRotatef(12.0f, 0.0f, 1.0f, 0.0f); // turn inwards slightly towards screen
        glRotatef(-15.0f, 0.0f, 0.0f, 1.0f); // tilt down towards audience
        drawSpeakerDetailed(0.5f, 1.6f, 1.0f, true);
        glPopMatrix();

        // Right wall speaker
        glPushMatrix();
        glTranslatef(kWallFixtureX, yHeight, z);
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
    drawBlock(0.0f, -0.2f, 0.0f, kRoomWidth, 0.6f, kRoomDepth);

    drawSimpleWalls();
    drawGate(gCeilingLightsVisible, false); // Right wall EXIT
    drawGate(gCeilingLightsVisible, true);  // Left wall gate
    drawCeilingSurface();
    drawWallSpeakers();

    for (int plank = 0; plank < 18; ++plank) {
        float z = -20.0f + plank * 2.3f;
        float n = noise2D(plank, 77);
        setMaterial(0.24f + n * 0.08f, 0.17f + n * 0.05f, 0.12f + n * 0.05f, 20.0f, 0.10f);
        drawBlock(0.0f, 0.02f, z, kFloorPlankWidth, 0.06f, 1.9f);
    }

    drawCeilingLights();
}

void setShadowPassMode(bool inPass) {
    gInShadowPass = inPass;
}

bool isInShadowPass() {
    return gInShadowPass;
}

void toggleCeilingLights() {
    gCeilingLightsVisible = !gCeilingLightsVisible;
}

bool areCeilingLightsVisible() {
    return gCeilingLightsVisible;
}
