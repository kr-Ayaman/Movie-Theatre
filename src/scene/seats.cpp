#include "scene/seats.h"

#include "render/lighting.h"
#include "render/primitives.h"

namespace {

const float kSeatStartZ = 0.0f;
const float kRowSpacing = 1.55f;
const float kSeatStartY = 1.1f;
const float kRowRise = 0.78f;

void drawSingleSeat(float x, float y, float z) {
    const float stepTopY = y + 0.02f;
    // Raise the seat significantly so it attaches near the top of the riser wall
    const float seatBaseY = stepTopY + 0.70f;

    // The vertical riser of the step *behind* this seat is located exactly at z + 0.79.
    // We extend the base and backrest all the way to this riser for a perfect, clean cantilever mount.
    const float riserZ = z + 0.79f;

    // Base depth extended from front to completely flush against the riser wall
    const float baseDepth = 0.78f;
    const float baseZ = riserZ - (baseDepth / 2.0f);

    // Lifted seat base: underside remains fully open.
    setMaterial(0.78f, 0.12f, 0.10f, 24.0f, 0.20f);
    drawBlock(x, seatBaseY, baseZ, 0.84f, 0.16f, baseDepth);

    // Straight backrest firmly attached to the riser.
    const float backrestDepth = 0.16f;
    const float backrestZ = riserZ - (backrestDepth / 2.0f);
    setMaterial(0.74f, 0.11f, 0.10f, 24.0f, 0.20f);
    drawBlock(x, seatBaseY + 0.50f, backrestZ, 0.80f, 0.94f, backrestDepth);

    // Standard floating armrests extending straight back to the riser.
    setMaterial(0.30f, 0.12f, 0.06f, 15.0f, 0.05f); 
    // Left armrest
    drawBlock(x - 0.44f, seatBaseY + 0.35f, baseZ, 0.08f, 0.10f, baseDepth);
    // Right armrest
    drawBlock(x + 0.44f, seatBaseY + 0.35f, baseZ, 0.08f, 0.10f, baseDepth);
}

void drawSeatingSection(float startX, int cols) {
    const int rows = 12;
    for (int row = 0; row < rows; ++row) {
        float zPos = kSeatStartZ + row * kRowSpacing;
        float yPos = kSeatStartY + row * kRowRise;
        float sectionWidth = cols * 1.15f + 0.35f;
        float sectionCenterX = startX + (cols - 1) * 1.15f * 0.5f;

        setMaterial(0.33f, 0.33f, 0.34f, 9.0f, 0.05f);
        drawBlock(sectionCenterX, yPos - 0.45f, zPos, sectionWidth, 0.94f, 1.52f);

        for (int col = 0; col < cols; ++col) {
            float xPos = startX + col * 1.15f;
            drawSingleSeat(xPos, yPos, zPos);
        }

        // Draw a back wall for the very last row so it has something to attach to
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
