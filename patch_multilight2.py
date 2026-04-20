import re

with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

vshader_search = 'const char* kVertexShaderSource = R"('
vshader_start = text.find(vshader_search)
vshader_end = text.find(')";', vshader_start) + 3
vshader = text[vshader_start:vshader_end]

new_vshader = vshader.replace('varying vec4 vLightSpacePos;', 'varying vec4 vLightSpacePos;\nvarying vec3 vWorldPos;\nvarying vec3 vWorldNormal;\nvarying vec3 vWorldEyePos;')
new_vshader = new_vshader.replace('vWorldPos = worldPos.xyz;', 'vWorldPos = worldPos.xyz;\n    vWorldNormal = (uInverseViewMatrix * vec4(vNormal, 0.0)).xyz;\n    vWorldEyePos = (uInverseViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;')

text = text.replace(vshader, new_vshader)

fshader_search = 'const char* kFragmentShaderSource = R"('
fshader_start = text.find(fshader_search)
fshader_end = text.find(')";', fshader_start) + 3
fshader = text[fshader_start:fshader_end]

new_fshader = fshader.replace('varying vec4 vLightSpacePos;', 'varying vec4 vLightSpacePos;\nvarying vec3 vWorldPos;\nvarying vec3 vWorldNormal;\nvarying vec3 vWorldEyePos;')


accum_func_target = 'void accumulateLight('
acc_start = new_fshader.find(accum_func_target)
acc_end = new_fshader.find('void main() {')

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
        // Standard light processing for other lights (like screen)
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

    // --- 77 CEILING LIGHTS ---
    // Check if ceiling lights are turned off by looking at ambient color
    if (length(gl_LightSource[0].ambient.rgb) < 0.01) {
        return; 
    }

    // Grid boundaries
    float xMin = -15.2;
    float xMax = 15.2;
    float zMin = -20.6;
    float zMax = 20.6;
    float y = 20.2;
    
    vec3 l_amb = gl_LightSource[0].ambient.rgb * gl_FrontMaterial.ambient.rgb / 77.0;
    vec3 l_diff = gl_LightSource[0].diffuse.rgb * gl_FrontMaterial.diffuse.rgb / 77.0;
    vec3 l_spec = gl_LightSource[0].specular.rgb * gl_FrontMaterial.specular.rgb / 77.0;
    
    vec3 wN = normalize(vWorldNormal);
    if (!gl_FrontFacing) { wN = -wN; }
    vec3 wV = normalize(vWorldEyePos - vWorldPos);
    
    vec3 sumAmbient = vec3(0.0);
    vec3 sumDiffuse = vec3(0.0);
    vec3 sumSpecular = vec3(0.0);
    
    float occlusion = 1.0 - (shadowFactor * 0.85);

    // Using 77 points is perfectly fast for a fragment shader
    for (int row = 0; row < 7; ++row) {
        float lz = mix(zMin, zMax, float(row) / 6.0);
        for (int col = 0; col < 11; ++col) {
            float lx = mix(xMin, xMax, float(col) / 10.0);
            
            vec3 to_light = vec3(lx, y, lz) - vWorldPos;
            float dist = length(to_light);
            // Attenuation constants tuned for 77 lights over the room scale
            float atten = 1.0 / (1.0 + 0.03 * dist + 0.005 * dist * dist);
            
            vec3 L = to_light / dist;
            float ndotl = max(dot(wN, L), 0.0);
            
            sumAmbient += l_amb * atten;
            sumDiffuse += l_diff * ndotl * atten * occlusion;
            
            if (ndotl > 0.0) {
                vec3 wR = reflect(-L, wN);
                float specPower = pow(max(dot(wR, wV), 0.0), gl_FrontMaterial.shininess);
                sumSpecular += l_spec * specPower * atten * occlusion;
            }
        }
    }
    
    ambientAcc += vec4(sumAmbient, 0.0);
    diffuseAcc += vec4(sumDiffuse, 0.0);
    specularAcc += vec4(sumSpecular, 0.0);
}
'''
new_fshader = new_fshader[:acc_start] + accum_logic + new_fshader[acc_end:]

text = text.replace(fshader, new_fshader)

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)

