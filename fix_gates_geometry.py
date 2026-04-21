with open("src/scene/room.cpp", "r") as f:
    text = f.read()

start_str = "void drawGate(bool lit, bool isExit, bool isLeftWall) {"
end_str = "\nvoid drawWallSpeakers() {"

start_idx = text.find(start_str)
end_idx = text.find(end_str)

if start_idx != -1 and end_idx != -1:
    new_gate_code = """void drawGate(bool lit, bool isExit, bool isLeftWall) {
    float wallX = isLeftWall ? -16.8f : 16.8f; 
    float doorZ = 10.0f; // Align both precisely in front of each other 

    setSceneShaderEffect(kSceneShaderEffectDefault);

    // Frame (Pillars and Lintel)
    setMaterial(0.06f, 0.06f, 0.08f, 15.0f, 0.08f, 0.05f);
    // Left Pillar 
    drawBlock(wallX, 3.0f, doorZ - 1.8f, 0.6f, 6.0f, 0.4f);
    // Right Pillar 
    drawBlock(wallX, 3.0f, doorZ + 1.8f, 0.6f, 6.0f, 0.4f);
    // Top Lintel
    drawBlock(wallX, 5.7f, doorZ, 0.6f, 0.6f, 4.0f);

    // The double doors (recessed slightly to look like a real exit pathway)
    float doorX = isLeftWall ? -17.2f : 17.2f;
    setMaterial(0.03f, 0.03f, 0.04f, 10.0f, 0.03f, 0.02f); // very dark door surface
    drawBlock(doorX, 2.7f, doorZ, 0.2f, 5.4f, 3.2f);
    
    // Door pushbars (shiny metal handles)
    setMaterial(0.4f, 0.4f, 0.45f, 50.0f, 0.6f, 0.1f);
    float barX = isLeftWall ? -16.95f : 16.95f;
    drawBlock(barX, 2.7f, doorZ - 0.8f, 0.1f, 0.08f, 1.2f);
    drawBlock(barX, 2.7f, doorZ + 0.8f, 0.1f, 0.08f, 1.2f);

    // Center vertical divider for the double doors
    setMaterial(0.01f, 0.01f, 0.01f, 5.0f, 0.0f, 0.0f);
    drawBlock(doorX + (isLeftWall ? 0.05f : -0.05f), 2.7f, doorZ, 0.22f, 5.4f, 0.05f);

    // Small black Sign Box projecting from the lintel
    float signX = isLeftWall ? -16.6f : 16.6f;
    float signY = 6.3f; // mounted above the doorway
    float signZ = doorZ;
    setMaterial(0.01f, 0.01f, 0.01f, 4.0f, 0.02f, 0.0f);
    drawBlock(signX, signY, signZ, 0.1f, 0.5f, 1.8f);

    // Translate and properly scale down the neon text structure
    glPushMatrix();
    glTranslatef(isLeftWall ? signX + 0.06f : signX - 0.06f, signY, signZ);
    glScalef(0.30f, 0.30f, 0.30f); // Make the letters much smaller!
    
    float gap = 0.8f;
    float r = isExit ? 0.98f : 0.1f;
    float g = isExit ? 0.1f : 0.98f;
    float b = 0.1f;

    if (isExit) {
        float startZ = -1.2f;
        drawSignLetterE(0, 0, startZ, lit, r, g, b);
        drawSignLetterX(0, 0, startZ + gap, lit, r, g, b);
        drawSignLetterI(0, 0, startZ + gap * 2, lit, r, g, b);
        drawSignLetterT(0, 0, startZ + gap * 3, lit, r, g, b);
    } else {
        float startZ = -1.6f;
        drawSignLetterE(0, 0, startZ, lit, r, g, b);
        drawSignLetterN(0, 0, startZ + gap, lit, r, g, b);
        drawSignLetterT(0, 0, startZ + gap * 2, lit, r, g, b);
        drawSignLetterR(0, 0, startZ + gap * 3, lit, r, g, b);
        drawSignLetterY(0, 0, startZ + gap * 4, lit, r, g, b);
    }
    glPopMatrix();
}
"""
    text = text[:start_idx] + new_gate_code + text[end_idx:]
    with open("src/scene/room.cpp", "w") as f:
        f.write(text)
    print("Replaced gates geometry!")
else:
    print(f"Failed to find match. start={start_idx} end={end_idx}")
