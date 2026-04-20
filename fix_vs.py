with open("src/render/shader.cpp", "r") as f:
    text = f.read()

replacement = """
    // Calculate light space position using world position
    vec4 worldPos = uInverseViewMatrix * eyePos;
    vLightSpacePos = uLightSpaceMatrix * worldPos;
    
    vWorldPos = worldPos.xyz;
    vWorldEyePos = (uInverseViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vWorldNormal = normalize(mat3(uInverseViewMatrix[0].xyz, uInverseViewMatrix[1].xyz, uInverseViewMatrix[2].xyz) * vNormal);
    
    gl_Position = ftransform();
"""

import re
text = re.sub(r'// Calculate light space position using world position\s*vec4 worldPos = uInverseViewMatrix \* eyePos;\s*vLightSpacePos = uLightSpaceMatrix \* worldPos;\s*gl_Position = ftransform\(\);', replacement.strip(), text, count=1)

with open("src/render/shader.cpp", "w") as f:
    f.write(text)
