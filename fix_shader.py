with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

import re
# Find all occurrences of setSceneShaderEffect
pattern = r'void setSceneShaderEffect\(int effectMode\) \{[\s\S]*?\}'
matches = list(re.finditer(pattern, text))

if len(matches) > 1:
    # Keep the first one and remove everything after its end and up to the second one's end
    first_end = matches[0].end()
    second_start = matches[1].start()
    second_end = matches[1].end()
    
    # Actually just rebuild the setters part properly
    pass

