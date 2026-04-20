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
    auto sign = [](float val) { return (val > 0.0f) ? 1.0f : ((val < 0.0f) ? -1.0f : 0.0f); };
    auto spow = [sign](float val, float p) { return sign(val) * std::pow(std::fabs(val), p); };

    float hw = width * 0.5f;
    float hh = height * 0.5f;
    float hd = depth * 0.5f;
    float n = 0.25f; // Roundness exponent: 1.0 is sphere, 0.0 is perfect box
    int res = 20;

    glPushMatrix();
    glTranslatef(x, y, z);
    
    // Explicit white color to force fragment shader's lighting branch
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    for (int i = 0; i < res; ++i) {
        float u1 = -M_PI / 2.0f + (float)i / res * M_PI;
        float u2 = -M_PI / 2.0f + (float)(i + 1) / res * M_PI;

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= res * 2; ++j) {
            // Reverse v traversal to ensure CCW winding order! 
            // Important so gl_FrontFacing is true and normals point towards light!
            float v = M_PI - (float)j / (res * 2) * 2.0f * M_PI;

            for (int k = 0; k < 2; ++k) {
                float u = (k == 0) ? u2 : u1;
                float cu = std::cos(u);
                float su = std::sin(u);
                float cv = std::cos(v);
                float sv = std::sin(v);

                // Fix precision issues at the poles that cause star-like shadow artifacts
                if (std::abs(cu) < 1e-4f) { cu = 0.0f; }

                float nx = spow(cu, 2.0f - n) * spow(cv, 2.0f - n) / hw;
                float ny = spow(su, 2.0f - n) / hh;
                float nz = spow(cu, 2.0f - n) * spow(sv, 2.0f - n) / hd;

                float len = std::sqrt(nx*nx + ny*ny + nz*nz);
                if (len > 0.0001f) {
                    glNormal3f(nx/len, ny/len, nz/len);
                } else {
                    glNormal3f(0.0f, 1.0f, 0.0f);
                }

                float px = hw * spow(cu, n) * spow(cv, n);
                float py = hh * spow(su, n);
                float pz = hd * spow(cu, n) * spow(sv, n);

                glVertex3f(px, py, pz);
            }
        }
        glEnd();
    }
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
    glTranslatef(xCenter + xShift, seatBaseY + 0.34f, (frontZ + rearZ) * 0.5f);
    if (isEdge) {
        glRotatef(edgeTilt, 0.0f, 0.0f, 1.0f);
    }
    drawArmrestTrapezoid(
        -0.35f,
        0.10f,
        0.10f,
        -armDepth * 0.36f,
        armDepth * 0.5f,
        -armDepth * 0.14f,
        armDepth * 0.14f,
        0.11f,
        0.08f
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
    // Increased base width and depth to stretch the seat out further, while leaving room for armrests
    drawRoundedCushion(x, seatBaseY + 0.02f, baseZ - 0.05f, 0.86f, 0.28f, 0.98f);

    const float backrestDepth = 0.18f;
    const float backrestZ = riserZ - (backrestDepth / 2.0f);
    setMaterial(0.74f, 0.11f, 0.10f, 24.0f, 0.20f);
    drawRoundedCushion(x, seatBaseY + 0.56f, backrestZ, 0.84f, 1.02f, 0.30f);

    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawSeatingSection(float startX, int cols) {
    const int rows = 14;
    for (int row = 0; row < rows; ++row) {
        float zPos = kSeatStartZ + row * kRowSpacing;
        float yPos = kSeatStartY + row * kRowRise;
        float sectionWidth = cols * kSeatSpacing + 0.60f;
        float sectionCenterX = startX + (cols - 1) * kSeatSpacing * 0.5f;

        // Apply a dark grey architectural tile/brick shader to the stadium riser structure
        setSceneShaderEffect(kSceneShaderEffectBrick);
        setMaterial(0.20f, 0.21f, 0.22f, 12.0f, 0.08f);
        float riserTop = yPos + 0.02f; drawBlock(sectionCenterX, riserTop / 2.0f, zPos, sectionWidth, riserTop, 1.52f);
        setSceneShaderEffect(kSceneShaderEffectDefault);

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
            
            // Apply the same shader to the final back wall of the seating structure
            setSceneShaderEffect(kSceneShaderEffectBrick);
            setMaterial(0.20f, 0.21f, 0.22f, 12.0f, 0.08f);
            drawBlock(sectionCenterX, nextYPos - 0.45f, nextZPos, sectionWidth, 0.94f, 1.52f);
            setSceneShaderEffect(kSceneShaderEffectDefault);
        }
    }
}

void drawAisle(float centerX, float width) {
    // The previous brick shader aggressively darkens materials by 0.3x.
    // Switching to the Cushion shader so it keeps the material's actual brightness while looking like a woven carpet.
    setSceneShaderEffect(kSceneShaderEffectCushion); 
    for (int row = 0; row < 14; ++row) {
        float zPos = kSeatStartZ + row * kRowSpacing;
        float yPos = kSeatStartY + row * kRowRise;

        // Instead of one big block, draw 2 stairs per row
        float stepDepth = kRowSpacing / 2.0f;
        float stepRise = kRowRise / 2.0f;
        
        for (int step = 0; step < 2; ++step) {
            float stepZ = zPos - (kRowSpacing / 2.0f) + stepDepth / 2.0f + step * stepDepth;
            float stepY = yPos - kRowRise + stepRise + step * stepRise - 0.44f;
            
            // Deep navy/midnight blue carpet color, bright enough to be visible and not pitch black
            setMaterial(0.12f, 0.15f, 0.25f, 10.0f, 0.04f);
            float stairTop = stepY + 0.45f; drawBlock(centerX, stairTop / 2.0f, stepZ, width, stairTop, stepDepth);
        }

        // Keep the little aisle lights
        if (row % 2 == 0) {
            setSceneShaderEffect(kSceneShaderEffectDefault);
            setMaterial(1.0f, 0.86f, 0.36f, 45.0f, 0.35f, 0.50f);
            drawBlock(centerX, yPos - 0.02f, zPos + 0.58f, 0.20f, 0.05f, 0.08f);
            setSceneShaderEffect(kSceneShaderEffectCushion);
        }

        
    }
    setSceneShaderEffect(kSceneShaderEffectDefault);
}

}  // namespace

void drawSeats() {
    // Added 1 seat to the far left (cols 6->7, startX -12.6 -> -13.6)
    drawSeatingSection(-13.6f, 7);
    drawSeatingSection(-4.25f, 9);
    // Added 1 seat to the far right (cols 6->7, startX 7.2 remains same, expands right)
    drawSeatingSection(7.2f, 7);

    // Dynamic width for Aisles - utilizing the full vacant space!
    // Left aisle gap: -6.8 to -5.05. Width = 1.75. Center = -5.925.
    drawAisle(-5.925f, 1.75f);
    
    // Right aisle gap: 4.55 to 6.4. Width = 1.85. Center = 5.475.
    drawAisle(5.475f, 1.85f);
    
    // Far left wall stairs. Room inner wall is ~-16.7. Leftmost block edge is -14.4. Gap=2.3, center=-15.55.
    drawAisle(-15.55f, 2.3f);

    // Far right wall stairs. Rightmost block edge is 14.0. Room inner wall is ~16.7. Gap=2.7, center=15.35.
    drawAisle(15.35f, 2.7f);

    // Fill the empty void between the last row and the back wall
    setSceneShaderEffect(kSceneShaderEffectDefault);
    setMaterial(0.12f, 0.15f, 0.25f, 10.0f, 0.04f); 
    drawBlock(0.0f, 11.26f / 2.0f, 21.45f, 34.0f, 11.26f, 1.15f);
}
