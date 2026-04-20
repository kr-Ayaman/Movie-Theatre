with open("src/scene/seats.cpp", "r") as f:
    text = f.read()

# Remove the broken 'back wall for the aisle' block in drawAisle
import re
text = re.sub(r'// Draw back wall for the aisle at the last row.*?if \(row == 13\) \{[^\}]+\}', '', text, flags=re.DOTALL)

# Add a giant filler block behind the stadium
filler = """
    // Fill the empty void between the last row and the back wall
    setSceneShaderEffect(kSceneShaderEffectDefault);
    setMaterial(0.12f, 0.15f, 0.25f, 10.0f, 0.04f); // Match the dark carpet material
    // Last row Z block ends roughly at 20.91. Wall is at 22.0. Center = 21.455, Depth = 1.1
    // The top of the last row block is ~ y=10.79. So center Y is ~5.4, Height = 10.8
    drawBlock(0.0f, 5.0f, 21.5f, 34.0f, 11.5f, 1.2f);
}"""
text = text.replace("    drawAisle(15.35f, 2.7f);\n}", "    drawAisle(15.35f, 2.7f);\n" + filler)

with open("src/scene/seats.cpp", "w") as f:
    f.write(text)

