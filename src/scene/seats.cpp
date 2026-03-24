#include "scene/seats.h"

#include "render/lighting.h"
#include "render/primitives.h"

namespace {

void drawSingleSeat(float x, float y, float z) {
    setMaterial(0.78f, 0.12f, 0.10f, 24.0f, 0.20f);
    drawBlock(x, y, z, 0.88f, 0.25f, 0.90f);

    setMaterial(0.74f, 0.11f, 0.10f, 24.0f, 0.20f);
    drawBlock(x, y + 0.63f, z + 0.34f, 0.88f, 1.05f, 0.22f);

    setMaterial(0.10f, 0.10f, 0.10f, 8.0f, 0.05f);
    drawBlock(x - 0.45f, y + 0.22f, z, 0.08f, 0.36f, 0.82f);
    drawBlock(x + 0.45f, y + 0.22f, z, 0.08f, 0.36f, 0.82f);
}

void drawSeatingSection(float startX, int cols) {
    const int rows = 12;
    for (int row = 0; row < rows; ++row) {
        float zPos = -4.2f + row * 1.55f;
        float yPos = 1.1f + row * 0.78f;
        float sectionWidth = cols * 1.15f + 0.35f;
        float sectionCenterX = startX + (cols - 1) * 1.15f * 0.5f;

        setMaterial(0.33f, 0.33f, 0.34f, 9.0f, 0.05f);
        drawBlock(sectionCenterX, yPos - 0.45f, zPos, sectionWidth, 0.94f, 1.52f);

        for (int col = 0; col < cols; ++col) {
            float xPos = startX + col * 1.15f;
            drawSingleSeat(xPos, yPos, zPos);
        }
    }
}

void drawAisle(float centerX) {
    for (int row = 0; row < 12; ++row) {
        float zPos = -4.2f + row * 1.55f;
        float yPos = 1.1f + row * 0.78f;

        setMaterial(0.35f, 0.35f, 0.36f, 9.0f, 0.05f);
        drawBlock(centerX, yPos - 0.44f, zPos, 1.05f, 0.90f, 1.52f);

        if (row % 2 == 0) {
            setMaterial(1.0f, 0.86f, 0.36f, 45.0f, 0.35f, 0.50f);
            drawBlock(centerX, yPos - 0.02f, zPos + 0.58f, 0.20f, 0.05f, 0.08f);
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
