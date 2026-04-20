import re

with open("src/scene/seats.cpp", "r") as f:
    text = f.read()

# Fix drawSeatingSection riser block to reach the floor
text = text.replace(
    'drawBlock(sectionCenterX, yPos - 0.45f, zPos, sectionWidth, 0.94f, 1.52f);',
    'float riserTop = yPos + 0.02f; drawBlock(sectionCenterX, riserTop / 2.0f, zPos, sectionWidth, riserTop, 1.52f);'
)

# Fix drawAisle stair blocks to reach the floor
text = text.replace(
    'drawBlock(centerX, stepY, stepZ, width, 0.90f, stepDepth);',
    'float stairTop = stepY + 0.45f; drawBlock(centerX, stairTop / 2.0f, stepZ, width, stairTop, stepDepth);'
)

# Fix the filler block at the end
old_filler = """// Fill the empty void between the last row and the back wall
    setSceneShaderEffect(kSceneShaderEffectDefault);
    setMaterial(0.12f, 0.15f, 0.25f, 10.0f, 0.04f); // Match the dark carpet material
    // Last row Z block ends roughly at 20.91. Wall is at 22.0. Center = 21.455, Depth = 1.1
    // The top of the last row block is ~ y=10.79. So center Y is ~5.4, Height = 10.8
    drawBlock(0.0f, 5.0f, 21.5f, 34.0f, 11.5f, 1.2f);"""

# Let's calculate the exact top: 
# Row 13 yPos = 1.1 + 13*0.78 = 11.24. Riser top = 11.26.
# Let's set it to 11.26 height, Y center 5.63.
new_filler = """// Fill the empty void between the last row and the back wall
    setSceneShaderEffect(kSceneShaderEffectDefault);
    setMaterial(0.12f, 0.15f, 0.25f, 10.0f, 0.04f); 
    drawBlock(0.0f, 11.26f / 2.0f, 21.45f, 34.0f, 11.26f, 1.15f);"""
    
if old_filler in text:
    text = text.replace(old_filler, new_filler)
else:
    # try Regex
    text = re.sub(r'// Fill the empty void.*?drawBlock\([^;]+;', new_filler, text, flags=re.DOTALL)

with open("src/scene/seats.cpp", "w") as f:
    f.write(text)

