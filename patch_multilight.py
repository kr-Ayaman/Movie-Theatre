import re

with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

# Add vWorldPos to varyings
vshader = text[text.find('const char* kVertexShaderSource'):text.find(')";', text.find('const char* kVertexShaderSource'))+3]
new_vshader = vshader.replace('varying vec4 vLightSpacePos;', 'varying vec4 vLightSpacePos;\nvarying vec3 vWorldPos;')
new_vshader = new_vshader.replace('vec4 worldPos = uInverseViewMatrix * eyePos;\n    vLightSpacePos = uLightSpaceMatrix * worldPos;', 'vec4 worldPos = uInverseViewMatrix * eyePos;\n    vLightSpacePos = uLightSpaceMatrix * worldPos;\n    vWorldPos = worldPos.xyz;')

text = text.replace(vshader, new_vshader)

fshader = text[text.find('const char* kFragmentShaderSource'):text.find(')";', text.find('const char* kFragmentShaderSource'))+3]
new_fshader = fshader.replace('varying vec4 vLightSpacePos;', 'varying vec4 vLightSpacePos;\nvarying vec3 vWorldPos;')

# Replace accumulateLight with a version that does the 77 grid lights logic
accum_func_target = 'void accumulateLight('
acc_end = new_fshader.find('void main() {')

accum_logic = '''
void accumulateLight(
    int lightIndex,
    vec3 N,
    vec3 V,
    inout vec4 ambientAcc,
    inout vec4 diffuseAcc,
    inout vec4 specularAcc,
    float shadowFactor
) {
    if (lightIndex != 0) {
        // Standard light processing for other lights like screen light
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
    
    // Check if ceiling lights are turned off by looking at ambient color
    if (length(gl_LightSource[0].ambient.rgb) < 0.01) {
        return; 
    }

    // Grid boundaries
    float xMin = -15.2;
    float xMax = 15.2;
    float zMin = -20.6;
    float zMax = 20.6;
    float y = 20.72;
    
    vec3 gridAmbient = vec3(0.0);
    vec3 gridDiffuse = vec3(0.0);
    vec3 gridSpecular = vec3(0.0);
    
    // Apply center shadow map estimate as a general occlusion/ambient factor 
    float occlusion = 1.0 - shadowFactor * 0.7; // Shadows dim lights by 70%
    
    for (int row = 0; row < 7; ++row) {
        float lz = mix(zMin, zMax, float(row) / 6.0);
        for (int col = 0; col < 11; ++col) {
            float lx = mix(xMin, xMax, float(col) / 10.0);
            
            // Light position in world space
            vec3 lpWorld = vec3(lx, y, lz);
            vec3 to_light = lpWorld - vWorldPos;
            float dist = length(to_light);
            vec3 L_world = to_light / dist;
            
            // Convert L_world to eye space to match N and V (which are in eye space)
            // L_eye = inverse_transpose(View) * L_world, but since view has no scaling, 
            // View rotation matrix * L_world is enough.
            // Using a simple trick: transform direction by mat3(gl_ModelViewMatrix) is WRONG because 
            // gl_ModelViewMatrix includes model transform! We need just the view matrix. 
            // But actually we have uInverseViewMatrix. So ViewMatrix is inverse(uInverseViewMatrix).
            // Let's just do math directly in EYE space instead!
            
            // World light pos transformed to Eye space:
            // Since we know gl_LightSource[0].position is NOT what we want here, 
            // we can transform world pos with uInverseViewMatrix? No, uInverseViewMatrix converts Eye->World.
            // So we need ViewMatrix to convert World->Eye.
            // We can also just do lighting in world space! V and N can be brought to world space.
        }
    }
}
'''

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)

