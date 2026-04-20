import re

with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

# Replace the inner loop logic to simulate downwards recessed lights (spotlights)
target_logic_old = """            vec3 lpWorld = vec3(lx, y, lz);
            vec3 toLight = lpWorld - vWorldPos;
            float dist2 = dot(toLight, toLight);
            float dist = sqrt(dist2);
            vec3 L_world = toLight / dist;

            float ndotl = max(dot(worldN, L_world), 0.0);
            if (ndotl > 0.0) {
                // Point light attenuation
                float atten = 1.0 / (1.0 + 0.02 * dist + 0.005 * dist2);
                gridDiffuse += ndotl * atten * shadowMult;

                vec3 R_world = reflect(-L_world, worldN);
                float spec = pow(max(dot(R_world, worldV), 0.0), gl_FrontMaterial.shininess);
                gridSpecular += spec * atten * shadowMult;
            }"""

target_logic_new = """            vec3 lpWorld = vec3(lx, y, lz);
            vec3 toLight = lpWorld - vWorldPos;
            float dist2 = dot(toLight, toLight);
            float dist = sqrt(dist2);
            vec3 L_world = toLight / dist;

            float ndotl = max(dot(worldN, L_world), 0.0);
            if (ndotl > 0.0) {
                // Make each ceiling light a wide downward spotlight to prevent harsh wall hotspots
                float spotEffect = max(dot(-L_world, vec3(0.0, 1.0, 0.0)), 0.0); // 1.0 straight down, 0.0 sideways
                
                // Only allow light to spread downwards within a wide cone
                spotEffect = smoothstep(0.4, 0.9, spotEffect); // softer edge
                
                // Gentler distance attenuation so the floor gets well lit and the walls are smooth
                float atten = 1.0 / (1.0 + 0.015 * dist + 0.002 * dist2);
                
                float totalLight = ndotl * atten * spotEffect * shadowMult;
                gridDiffuse += totalLight;

                vec3 R_world = reflect(-L_world, worldN);
                float spec = pow(max(dot(R_world, worldV), 0.0), gl_FrontMaterial.shininess);
                gridSpecular += spec * atten * spotEffect * shadowMult;
            }"""

text = text.replace(target_logic_old, target_logic_new)

# Increase ambient intensity
text = text.replace('ambientAcc += gl_LightSource[0].ambient * gl_FrontMaterial.ambient;', 'ambientAcc += gl_LightSource[0].ambient * gl_FrontMaterial.ambient * 1.5;')

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)

