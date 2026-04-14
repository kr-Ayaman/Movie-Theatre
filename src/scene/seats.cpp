#include "scene/seats.h"

#include "render/lighting.h"
#include "render/primitives.h"
#include "render/shader.h"

#include <GL/glut.h>

#include <cmath>

namespace {

const float kSeatStartZ = 0.0f;
const float kRowSpacing = 1.55f;
const float kSeatStartY = 1.1f;
const float kRowRise = 0.78f;
const float kSeatSpacing = 1.00f;

void drawRoundedCushion(float x, float y, float z, float width, float height, float depth) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(width * 0.5f, height * 0.5f, depth * 0.5f);
    glutSolidSphere(1.0, 32, 24);
    glPopMatrix();

    // Secondary lobe gives a plush cushion profile without introducing hard edges.
    glPushMatrix();
    glTranslatef(x, y - height * 0.12f, z - depth * 0.09f);
    glScalef(width * 0.40f, height * 0.24f, depth * 0.34f);
    glutSolidSphere(1.0, 28, 20);
    glPopMatrix();
}

void setFaceNormal(
    float ax,
    float ay,
    float az,
    float bx,
    float by,
    float bz,
    float cx,
    float cy,
    float cz
) {
    float ux = bx - ax;
    float uy = by - ay;
    float uz = bz - az;
    float vx = cx - ax;
    float vy = cy - ay;
    float vz = cz - az;

    float nx = uy * vz - uz * vy;
    float ny = uz * vx - ux * vz;
    float nz = ux * vy - uy * vx;

    float length = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (length > 1e-6f) {
        glNormal3f(nx / length, ny / length, nz / length);
    } else {
        glNormal3f(0.0f, 1.0f, 0.0f);
    }
}

void drawArmrestTrapezoid(
    float yBottom,
    float yTopFront,
    float yTopBack,
    float zFront,
    float zBack,
    float topFrontInset,
    float topBackInset,
    float bottomWidth,
    float topWidth
) {
    float xb0 = -bottomWidth * 0.5f;
    float xb1 = bottomWidth * 0.5f;
    float xt0 = -topWidth * 0.5f;
    float xt1 = topWidth * 0.5f;

    float ztf = zFront + topFrontInset;
    float ztb = zBack - topBackInset;

    glBegin(GL_QUADS);

    // Front face
    setFaceNormal(xb0, yBottom, zFront, xb1, yBottom, zFront, xt1, yTopFront, ztf);
    glVertex3f(xb0, yBottom, zFront);
    glVertex3f(xb1, yBottom, zFront);
    glVertex3f(xt1, yTopFront, ztf);
    glVertex3f(xt0, yTopFront, ztf);

    // Back face
    setFaceNormal(xb0, yBottom, zBack, xt0, yTopBack, ztb, xt1, yTopBack, ztb);
    glVertex3f(xb0, yBottom, zBack);
    glVertex3f(xt0, yTopBack, ztb);
    glVertex3f(xt1, yTopBack, ztb);
    glVertex3f(xb1, yBottom, zBack);

    // Left face
    setFaceNormal(xb0, yBottom, zFront, xb0, yBottom, zBack, xt0, yTopBack, ztb);
    glVertex3f(xb0, yBottom, zFront);
    glVertex3f(xb0, yBottom, zBack);
    glVertex3f(xt0, yTopBack, ztb);
    glVertex3f(xt0, yTopFront, ztf);

    // Right face
    setFaceNormal(xb1, yBottom, zFront, xt1, yTopFront, ztf, xt1, yTopBack, ztb);
    glVertex3f(xb1, yBottom, zFront);
    glVertex3f(xt1, yTopFront, ztf);
    glVertex3f(xt1, yTopBack, ztb);
    glVertex3f(xb1, yBottom, zBack);

    // Bottom face
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(xb0, yBottom, zFront);
    glVertex3f(xb0, yBottom, zBack);
    glVertex3f(xb1, yBottom, zBack);
    glVertex3f(xb1, yBottom, zFront);

    // Top face
    setFaceNormal(xt0, yTopFront, ztf, xt1, yTopFront, ztf, xt1, yTopBack, ztb);
    glVertex3f(xt0, yTopFront, ztf);
    glVertex3f(xt1, yTopFront, ztf);
    glVertex3f(xt1, yTopBack, ztb);
    glVertex3f(xt0, yTopBack, ztb);

    glEnd();
}

void drawRowArmrest(float xCenter, float seatBaseY, float baseZ, float backrestZ, float baseDepth, float sideSign, bool isEdge) {
    float frontZ = baseZ - baseDepth * 0.49f;
    float rearZ = backrestZ + 0.09f;
    float armDepth = (rearZ - frontZ) + 0.06f;

    float xShift = isEdge ? sideSign * 0.008f : 0.0f;
    float edgeTilt = 0.0f;

    setSceneShaderEffect(kSceneShaderEffectCushion);
    setMaterial(0.64f, 0.11f, 0.10f, 18.0f, 0.08f);

    // Clean trapezoidal armrest body matching side-view theatre profile.
    glPushMatrix();
    glTranslatef(xCenter + xShift, seatBaseY + 0.11f, (frontZ + rearZ) * 0.5f);
    if (isEdge) {
        glRotatef(edgeTilt, 0.0f, 0.0f, 1.0f);
    }
    drawArmrestTrapezoid(
        -0.12f,
        0.10f,
        0.10f,
        -armDepth * 0.36f,
        armDepth * 0.5f,
        -armDepth * 0.14f,
        armDepth * 0.14f,
        0.145f,
        0.095f
    );
    glPopMatrix();

    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawSingleSeat(float x, float y, float z) {
    const float stepTopY = y + 0.02f;
    const float seatBaseY = stepTopY + 0.70f;

    const float riserZ = z + 0.79f;

    const float baseDepth = 0.78f;
    const float baseZ = riserZ - (baseDepth / 2.0f);

    setSceneShaderEffect(kSceneShaderEffectCushion);
    setMaterial(0.78f, 0.12f, 0.10f, 24.0f, 0.20f);
    drawRoundedCushion(x, seatBaseY + 0.02f, baseZ, 0.86f, 0.28f, 0.82f);

    const float backrestDepth = 0.18f;
    const float backrestZ = riserZ - (backrestDepth / 2.0f);
    setMaterial(0.74f, 0.11f, 0.10f, 24.0f, 0.20f);
    drawRoundedCushion(x, seatBaseY + 0.56f, backrestZ, 0.84f, 1.02f, 0.30f);

    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawSeatingSection(float startX, int cols) {
    const int rows = 12;
    for (int row = 0; row < rows; ++row) {
        float zPos = kSeatStartZ + row * kRowSpacing;
        float yPos = kSeatStartY + row * kRowRise;
        float sectionWidth = cols * kSeatSpacing + 0.60f;
        float sectionCenterX = startX + (cols - 1) * kSeatSpacing * 0.5f;

        setMaterial(0.33f, 0.33f, 0.34f, 9.0f, 0.05f);
        drawBlock(sectionCenterX, yPos - 0.45f, zPos, sectionWidth, 0.94f, 1.52f);

        for (int col = 0; col < cols; ++col) {
            float xPos = startX + col * kSeatSpacing;
            drawSingleSeat(xPos, yPos, zPos);
        }

        // Row armrests: one between each seat pair plus one on both outer edges.
        float seatBaseY = yPos + 0.72f;
        float riserZ = zPos + 0.79f;
        float baseDepth = 0.78f;
        float baseZ = riserZ - (baseDepth / 2.0f);
        float backrestDepth = 0.18f;
        float backrestZ = riserZ - (backrestDepth / 2.0f);

        float armrestStartX = startX - 0.5f * kSeatSpacing;
        for (int arm = 0; arm <= cols; ++arm) {
            float xArm = armrestStartX + arm * kSeatSpacing;
            bool isEdge = (arm == 0 || arm == cols);
            float sideSign = (arm == 0) ? -1.0f : ((arm == cols) ? 1.0f : 0.0f);
            drawRowArmrest(xArm, seatBaseY, baseZ, backrestZ, baseDepth, sideSign, isEdge);
        }

        if (row == rows - 1) {
            float nextZPos = kSeatStartZ + (row + 1) * kRowSpacing;
            float nextYPos = kSeatStartY + (row + 1) * kRowRise;
            setMaterial(0.33f, 0.33f, 0.34f, 9.0f, 0.05f);
            drawBlock(sectionCenterX, nextYPos - 0.45f, nextZPos, sectionWidth, 0.94f, 1.52f);
        }
    }
}

void drawAisle(float centerX) {
    for (int row = 0; row < 12; ++row) {
        float zPos = kSeatStartZ + row * kRowSpacing;
        float yPos = kSeatStartY + row * kRowRise;

        setMaterial(0.35f, 0.35f, 0.36f, 9.0f, 0.05f);
        drawBlock(centerX, yPos - 0.44f, zPos, 1.05f, 0.90f, 1.52f);

        if (row % 2 == 0) {
            setMaterial(1.0f, 0.86f, 0.36f, 45.0f, 0.35f, 0.50f);
            drawBlock(centerX, yPos - 0.02f, zPos + 0.58f, 0.20f, 0.05f, 0.08f);
        }

        // Draw back wall for the aisle at the last row
        if (row == 11) {
            float nextZPos = kSeatStartZ + (row + 1) * kRowSpacing;
            float nextYPos = kSeatStartY + (row + 1) * kRowRise;
            setMaterial(0.35f, 0.35f, 0.36f, 9.0f, 0.05f);
            drawBlock(centerX, nextYPos - 0.44f, nextZPos, 1.05f, 0.90f, 1.52f);
        }
    }
}

}  // namespace

void drawSeats() {
    drawSeatingSection(-12.6f, 6);
    drawSeatingSection(-4.25f, 9);
    drawSeatingSection(7.2f, 6);

    drawAisle(-5.6f);
    drawAisle(6.1f);
}
