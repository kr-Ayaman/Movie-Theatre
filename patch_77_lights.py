import re

with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

# Make sure we have varying vec3 vWorldPos in vertex shader
v_shader_start = text.find('const char* kVertexShaderSource = R"(', 0)
v_shader_end = text.find(')";', v_shader_start) + 3
v_shader = text[v_shader_start:v_shader_end]
if 'varying vec3 vWorldPos;' not in v_shader:
    new_v_shader = v_shader.replace('varying vec4 vLightSpacePos;', 'varying vec4 vLightSpacePos;\nvarying vec3 vWorldPos;')
    # Handle both old and new possible variations
    if 'vec4 worldPos = uInverseViewMatrix * eyePos;' in new_v_shader:
        new_v_shader = new_v_shader.replace('vLightSpacePos = uLightSpaceMatrix * worldPos;', 'vLightSpacePos = uLightSpaceMatrix * worldPos;\n    vWorldPos = worldPos.xyz;')
    text = text.replace(v_shader, new_v_shader)

f_shader_start = text.find('const char* kFragmentShaderSource = R"(', 0)
f_shader_end = text.find(')";', f_shader_start) + 3
f_shader = text[f_shader_start:f_shader_end]

if 'varying vec3 vWorldPos;' not in f_shader:
    new_f_shader = f_shader.replace('varying vec4 vLightSpacePos;', 'varying vec4 vLightSpacePos;\nvarying vec3 vWorldPos;\nuniform mat4 uInverseViewMatrix;')
    text = text.replace(f_shader, new_f_shader)

# Re-read f_shader after first replacement
f_shader_start = text.find('const char* kFragmentShaderSource = R"(', 0)
f_shader_end = text.find(')";', f_shader_start) + 3
f_shader = text[f_shader_start:f_shader_end]

accum_logic = '''void accumulateLight(
    int lightIndex,
    vec3 N,
    vec3 V,
    inout vec4 ambientAcc,
    inout vec4 diffuseAcc,
    inout vec4 specularAcc,
    float shadowFactor
) {
    if (lightIndex != 0) {
        vec4 lightPos = gl_LightSource[lightIndex].position;
        vec3 L = normalize(lightPos.xyz - vEyePos * lightPos.w);

        float attenuation = 1.0;
        if (lightPos.w != 0.0) {
            float distanceToLight = length(lightPos.xyz - vEyePos);
            attenuation = 1.0 / (
                gl_LightSource[lightIndex].constantAttenuation +
                gl_LightSource[lightIndex].linearAttenuation * distanceToLight +
                gl_LightSource[lightIndex].quadraticAttenuation * distanceToLight * distanceToLight
            );
        }

        float spotFactor = 1.0;
        if (gl_LightSource[lightIndex].spotCutoff <= 90.0) {
            spotFactor = computeSpotFactor(lightIndex, L);
        }

        float ndotl = max(dot(N, L), 0.0);
        ambientAcc += gl_LightSource[lightIndex].ambient * gl_FrontMaterial.ambient * attenuation * spotFactor;
        diffuseAcc += gl_LightSource[lightIndex].diffuse * gl_FrontMaterial.diffuse * ndotl * attenuation * spotFactor;

        if (ndotl > 0.0) {
            vec3 R = reflect(-L, N);
            float specPower = pow(max(dot(R, V), 0.0), gl_FrontMaterial.shininess);
            specularAcc += gl_LightSource[lightIndex].specular * gl_FrontMaterial.specular * specPower * attenuation * spotFactor;
        }
        return;
    }

    // --- 77 CEILING LIGHTS --- (Calculated for lightIndex 0 only)
    if (length(gl_LightSource[0].diffuse.rgb) < 0.01) {
        return; // Ceiling lights are off
    }

    vec3 worldN = normalize(mat3(uInverseViewMatrix[0].xyz, uInverseViewMatrix[1].xyz, uInverseViewMatrix[2].xyz) * N);
    vec3 worldV = normalize(mat3(uInverseViewMatrix[0].xyz, uInverseViewMatrix[1].xyz, uInverseViewMatrix[2].xyz) * V);
    
    // Grid boundaries
    float xMin = -15.2;
    float xMax = 15.2;
    float zMin = -20.6;
    float zMax = 20.6;
    float y = 20.72;

    vec3 gridDiffuse = vec3(0.0);
    vec3 gridSpecular = vec3(0.0);

    // Apply shadow map to dim the lights overall where blocked
    float shadowMult = 1.0 - shadowFactor * 0.8;

    for (int row = 0; row < 7; ++row) {
        float lz = mix(zMin, zMax, float(row) / 6.0);
        for (int col = 0; col < 11; ++col) {
            float lx = mix(xMin, xMax, float(col) / 10.0);

            vec3 lpWorld = vec3(lx, y, lz);
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
            }
        }
    }

    // Scale down the sum and multiply by material properties
    float intensity = 5.0; // Global brightness scalar for the grid
    vec4 baseDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse;
    vec4 baseSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular;

    ambientAcc += gl_LightSource[0].ambient * gl_FrontMaterial.ambient;
    diffuseAcc += vec4(gridDiffuse * (intensity / 77.0), 0.0) * baseDiffuse;
    specularAcc += vec4(gridSpecular * (intensity / 77.0), 0.0) * baseSpecular;
}
'''
# Using regex to replace the old accumulateLight
pattern = r'void accumulateLight\([^\{]+\{[\s\S]*?(?=\nvoid main\(\))'
new_f_shader_replaced = re.sub(pattern, accum_logic + '\n', f_shader)

text = text.replace(f_shader, new_f_shader_replaced)

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)

