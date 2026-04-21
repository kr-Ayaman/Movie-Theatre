import re

with open("src/scene/room.cpp", "r") as f:
    text = f.read()

# We need to replace the letter functions and drawExitDoor
# First, let's find the start of drawSignLetterE and the end of drawExitDoor block

start_idx = text.find('void drawSignLetterE')
# find end of drawExitDoor
end_str = 'drawSignLetterT(letterX, letterTopY, letterZ0 + 3.05f, lit);\n}'
end_idx = text.find(end_str) + len(end_str)

if start_idx != -1 and end_idx != -1:
    new_code = """
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

void drawSignLetterN(float x, float y, float z, bool lit, float r, float g, float b) {
    setNeonMaterial(lit, r, g, b);
    drawBlock(x, y, z - 0.2f, 0.08f, 0.60f, 0.10f);
    drawBlock(x, y, z + 0.2f, 0.08f, 0.60f, 0.10f);
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-35.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.06f, 0.58f, 0.10f);
    glPopMatrix();
}

void drawSignLetterR(float x, float y, float z, bool lit, float r, float g, float b) {
    setNeonMaterial(lit, r, g, b);
    drawBlock(x, y, z - 0.15f, 0.08f, 0.60f, 0.10f);
    drawBlock(x, y + 0.25f, z + 0.05f, 0.08f, 0.10f, 0.30f);
    drawBlock(x, y + 0.15f, z + 0.20f, 0.08f, 0.20f, 0.10f);
    drawBlock(x, y + 0.05f, z + 0.05f, 0.08f, 0.10f, 0.30f);
    glPushMatrix();
    glTranslatef(x, y - 0.12f, z + 0.1f);
    glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.08f, 0.35f, 0.10f);
    glPopMatrix();
}

void drawSignLetterY(float x, float y, float z, bool lit, float r, float g, float b) {
    setNeonMaterial(lit, r, g, b);
    drawBlock(x, y - 0.15f, z, 0.08f, 0.30f, 0.10f);
    glPushMatrix();
    glTranslatef(x, y + 0.15f, z - 0.15f);
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.08f, 0.35f, 0.10f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(x, y + 0.15f, z + 0.15f);
    glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
    drawBlock(0.0f, 0.0f, 0.0f, 0.08f, 0.35f, 0.10f);
    glPopMatrix();
}

void drawGate(bool lit, bool isExit, bool isLeftWall) {
    float doorX = isLeftWall ? -16.45f : 16.45f;
    float doorY = 3.00f; // bottom sits on the ground (height 6, y=3)
    float doorZ = isExit ? 5.20f : -5.20f; 

    setSceneShaderEffect(kSceneShaderEffectDefault);
    // Gate arch framing
    setMaterial(0.08f, 0.08f, 0.10f, 12.0f, 0.06f, 0.05f);
    drawBlock(doorX, doorY, doorZ, 0.20f, 6.00f, 4.20f);

    // Inner door slightly darker
    setMaterial(0.04f, 0.05f, 0.06f, 8.0f, 0.04f, 0.08f);
    drawBlock(isLeftWall ? doorX + 0.05f : doorX - 0.05f, doorY, doorZ, 0.10f, 5.60f, 3.60f);

    // Sign background block
    float signX = isLeftWall ? doorX + 0.14f : doorX - 0.14f;
    float signY = doorY + 4.20f;
    float signZ = doorZ;
    setMaterial(0.02f, 0.02f, 0.02f, 4.0f, 0.03f, 0.0f);
    drawBlock(signX, signY, signZ, 0.08f, 1.00f, 4.00f);

    // Neon lights
    float letterX = isLeftWall ? signX + 0.08f : signX - 0.08f;
    float letterTopY = signY;
    float startZ = signZ - 1.20f;
    float gap = 0.8f;
    
    float r = isExit ? 0.98f : 0.1f;
    float g = isExit ? 0.1f : 0.98f;
    float b = 0.1f;

    if (isExit) {
        drawSignLetterE(letterX, letterTopY, startZ, lit, r, g, b);
        drawSignLetterX(letterX, letterTopY, startZ + gap, lit, r, g, b);
        drawSignLetterI(letterX, letterTopY, startZ + gap * 2, lit, r, g, b);
        drawSignLetterT(letterX, letterTopY, startZ + gap * 3, lit, r, g, b);
    } else {
        startZ = signZ - 1.6f;
        drawSignLetterE(letterX, letterTopY, startZ, lit, r, g, b);
        drawSignLetterN(letterX, letterTopY, startZ + gap, lit, r, g, b);
        drawSignLetterT(letterX, letterTopY, startZ + gap * 2, lit, r, g, b);
        drawSignLetterR(letterX, letterTopY, startZ + gap * 3, lit, r, g, b);
        drawSignLetterY(letterX, letterTopY, startZ + gap * 4, lit, r, g, b);
    }
}
"""
    text = text[:start_idx] + new_code + text[end_idx:]
    
    # replace drawExitDoor(gCeilingLightsVisible) with drawGate calls
    text = text.replace('drawExitDoor(gCeilingLightsVisible);', 
        'drawGate(gCeilingLightsVisible, true, false);  // Right wall EXIT\n    drawGate(gCeilingLightsVisible, false, true); // Left wall ENTRY')

    with open("src/scene/room.cpp", "w") as f:
        f.write(text)
    print("Replaced gates successfully!")
else:
    print("Could not find start/end bounds for gate replacement.")
