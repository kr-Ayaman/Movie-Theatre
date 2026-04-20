import re

with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

# Add uniforms mapping
if 'GLint gLightSpaceMatrixLoc = -1;' not in text:
    text = text.replace('GLint gEffectModeLocation = -1;', 'GLint gEffectModeLocation = -1;\nGLint gLightSpaceMatrixLoc = -1;\nGLint gShadowMapLoc = -1;\nGLint gInverseViewMatrixLoc = -1;')

# Replace Vertex Shader
v_shader_start = text.find('const char* kVertexShaderSource = R"(')
v_shader_end = text.find(')";', v_shader_start) + 3
if v_shader_start != -1 and v_shader_end != -1:
    old_v_shader = text[v_shader_start:v_shader_end]
    new_v_shader = '''const char* kVertexShaderSource = R"(
#version 120

varying vec3 vNormal;
varying vec3 vEyePos;
varying vec4 vColor;
varying vec3 vObjectPos;
varying vec3 vObjectNormal;
varying vec4 vLightSpacePos;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uInverseViewMatrix;

void main() {
    vec4 eyePos = gl_ModelViewMatrix * gl_Vertex;
    vEyePos = eyePos.xyz;
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    vColor = gl_Color;
    vObjectPos = gl_Vertex.xyz;
    vObjectNormal = normalize(gl_Normal);
    
    // Calculate light space position using world position
    vec4 worldPos = uInverseViewMatrix * eyePos;
    vLightSpacePos = uLightSpaceMatrix * worldPos;

    gl_Position = ftransform();
}
)";'''
    text = text.replace(old_v_shader, new_v_shader)

# Replace Fragment Shader
f_shader_start = text.find('const char* kFragmentShaderSource = R"(')
f_shader_end = text.find(')";', f_shader_start) + 3
if f_shader_start != -1 and f_shader_end != -1:
    old_f_shader = text[f_shader_start:f_shader_end]
    new_f_shader = '''const char* kFragmentShaderSource = R"(
#version 120

varying vec3 vNormal;
varying vec3 vEyePos;
varying vec4 vColor;
varying vec3 vObjectPos;
varying vec3 vObjectNormal;
varying vec4 vLightSpacePos;

uniform int uEffectMode;
uniform sampler2D uShadowMap;

float computeSpotFactor(int lightIndex, vec3 L) {
    vec3 spotDir = normalize(gl_LightSource[lightIndex].spotDirection);
    float spotCos = dot(-L, spotDir);
    if (spotCos < gl_LightSource[lightIndex].spotCosCutoff) {
        return 0.0;
    }
    return pow(max(spotCos, 0.0), gl_LightSource[lightIndex].spotExponent);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if (projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(2048.0, 2048.0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture2D(uShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void accumulateLight(
    int lightIndex,
    vec3 N,
    vec3 V,
    inout vec4 ambientAcc,
    inout vec4 diffuseAcc,
    inout vec4 specularAcc,
    float shadowFactor
) {
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
    
    // Only apply shadow to the primary light (lightIndex 0)
    float sf = (lightIndex == 0) ? shadowFactor : 0.0;

    ambientAcc += gl_LightSource[lightIndex].ambient * gl_FrontMaterial.ambient * attenuation * spotFactor;
    diffuseAcc += gl_LightSource[lightIndex].diffuse * gl_FrontMaterial.diffuse * ndotl * attenuation * spotFactor * (1.0 - sf);

    if (ndotl > 0.0) {
        vec3 R = reflect(-L, N);
        float specPower = pow(max(dot(R, V), 0.0), gl_FrontMaterial.shininess);
        specularAcc += gl_LightSource[lightIndex].specular * gl_FrontMaterial.specular * specPower * attenuation * spotFactor * (1.0 - sf);
    }
}

void main() {
    bool useVertexColor =
        (abs(vColor.r - 1.0) + abs(vColor.g - 1.0) + abs(vColor.b - 1.0) > 0.01) ||
        (vColor.a < 0.999);

    if (useVertexColor) {
        gl_FragColor = vColor;
        return;
    }

    vec3 N = normalize(vNormal);
    if (!gl_FrontFacing) {
        N = -N;
    }
    vec3 V = normalize(-vEyePos);

    // Calculate lightDir for shadow bias (approximating key light from view space)
    vec3 L0 = normalize(gl_LightSource[0].position.xyz - vEyePos * gl_LightSource[0].position.w);
    float shadow = calculateShadow(vLightSpacePos, N, L0);

    vec4 ambientAcc = gl_LightModel.ambient * gl_FrontMaterial.ambient;
    vec4 diffuseAcc = vec4(0.0);
    vec4 specularAcc = vec4(0.0);

    accumulateLight(0, N, V, ambientAcc, diffuseAcc, specularAcc, shadow);
    accumulateLight(1, N, V, ambientAcc, diffuseAcc, specularAcc, 0.0);
    accumulateLight(2, N, V, ambientAcc, diffuseAcc, specularAcc, 0.0);
    accumulateLight(3, N, V, ambientAcc, diffuseAcc, specularAcc, 0.0);

    vec4 litColor = gl_FrontMaterial.emission + ambientAcc + diffuseAcc + specularAcc;
    litColor.a = gl_FrontMaterial.diffuse.a;

    vec3 shadedRgb = litColor.rgb;

    if (uEffectMode == 1) {
        float foldWave = 0.5 + 0.5 * sin(vObjectPos.x * 45.0 + vObjectPos.z * 16.0);
        float verticalFade = 0.72 + 0.28 * (1.0 - abs(vObjectPos.y) * 1.5);
        float weave = 0.5 + 0.5 * sin(vObjectPos.y * 140.0) * sin(vObjectPos.x * 120.0);

        shadedRgb *= 0.72 + 0.35 * foldWave;
        shadedRgb *= verticalFade;
        shadedRgb += vec3(0.02, 0.01, 0.03) * weave;
    } else if (uEffectMode == 2) {
        vec3 absObjN = abs(normalize(vObjectNormal));
        vec2 uv;
        if (absObjN.x > absObjN.z && absObjN.x > absObjN.y) {
            uv = vObjectPos.zy;
        } else if (absObjN.z > absObjN.y) {
            uv = vObjectPos.xy;
        } else {
            uv = vObjectPos.xz;
        }

        vec2 tiledUv = uv * vec2(13.0, 9.2);
        vec2 tileCell = fract(tiledUv + 0.5);
        vec2 tileId = floor(tiledUv + 0.5);

        float edgeDistance = min(min(tileCell.x, 1.0 - tileCell.x), min(tileCell.y, 1.0 - tileCell.y));
        float groutMask = 1.0 - smoothstep(0.035, 0.068, edgeDistance);

        float variation = fract(sin(dot(tileId, vec2(13.137, 79.921))) * 24634.6345);
        float tileTone = 0.24 + variation * 0.09;
        float topDarkening = clamp(0.90 - (vObjectPos.y + 0.5) * 0.07, 0.62, 0.92);

        vec3 tileColor = shadedRgb * vec3(tileTone) * topDarkening;
        vec3 groutColor = vec3(0.07, 0.075, 0.085);
        shadedRgb = mix(tileColor, groutColor, groutMask);
    } else if (uEffectMode == 3) {
        vec3 absObjN = abs(normalize(vObjectNormal));
        vec2 uv;
        if (absObjN.x > absObjN.z && absObjN.x > absObjN.y) {
            uv = vObjectPos.zy;
        } else if (absObjN.z > absObjN.y) {
            uv = vObjectPos.xy;
        } else {
            uv = vObjectPos.xz;
        }

        vec2 weaveUv = uv * vec2(84.0, 112.0);
        float weave = 0.5 + 0.5 *
            sin(weaveUv.x + weaveUv.y * 0.09) *
            sin(weaveUv.y - weaveUv.x * 0.07);

        float loft = clamp(0.92 + (vObjectPos.y + 0.2) * 0.12, 0.86, 1.06);
        float rim = pow(clamp(1.0 - max(dot(N, V), 0.0), 0.0, 1.0), 1.5);

        shadedRgb *= loft;
        shadedRgb *= 0.95 + (weave - 0.5) * 0.10;
        shadedRgb -= vec3(0.08, 0.05, 0.05) * rim * 0.30;
    } else if (uEffectMode == 4) {
        vec2 uv = vObjectPos.xz * vec2(0.56, 0.34);
        vec2 panelUv = fract(uv + 0.5);
        vec2 panelId = floor(uv + 0.5);

        float edgeDistance = min(min(panelUv.x, 1.0 - panelUv.x), min(panelUv.y, 1.0 - panelUv.y));
        float gridMask = 1.0 - smoothstep(0.018, 0.045, edgeDistance);

        float panelNoise = fract(sin(dot(panelId, vec2(17.123, 41.987))) * 18653.4218);
        float panelTone = 0.11 + panelNoise * 0.05;
        vec3 panelColor = shadedRgb * vec3(panelTone);

        vec3 gridColor = vec3(0.24, 0.25, 0.27);
        shadedRgb = mix(panelColor, gridColor, gridMask * 0.92);
    }

    gl_FragColor = vec4(clamp(shadedRgb, 0.0, 1.0), litColor.a);
}
)";'''
    text = text.replace(old_f_shader, new_f_shader)

# Add setter functions if missing
if 'void setLightSpaceMatrix' not in text:
    funcs = """
void setLightSpaceMatrix(float* matrix) {
    if (gLightSpaceMatrixLoc != -1) {
        glUniformMatrix4fv(gLightSpaceMatrixLoc, 1, GL_FALSE, matrix);
    }
}

void setInverseViewMatrix(float* matrix) {
    if (gInverseViewMatrixLoc != -1) {
        glUniformMatrix4fv(gInverseViewMatrixLoc, 1, GL_FALSE, matrix);
    }
}

void setShadowMap(int texUnit) {
    if (gShadowMapLoc != -1) {
        glUniform1i(gShadowMapLoc, texUnit);
    }
}
"""
    # Insert before 'void setSceneShaderEffect(int effectMode)'
    idx = text.find('void setSceneShaderEffect(int effectMode)')
    if idx != -1:
        text = text[:idx] + funcs + text[idx:]


# Bind uniform locations
init_func = text.find('gEffectModeLocation = glGetUniformLocation(gShaderProgram, "uEffectMode");')
if init_func != -1 and 'gLightSpaceMatrixLoc = glGetUniformLocation' not in text:
    bindings = """gEffectModeLocation = glGetUniformLocation(gShaderProgram, "uEffectMode");
    gLightSpaceMatrixLoc = glGetUniformLocation(gShaderProgram, "uLightSpaceMatrix");
    gInverseViewMatrixLoc = glGetUniformLocation(gShaderProgram, "uInverseViewMatrix");
    gShadowMapLoc = glGetUniformLocation(gShaderProgram, "uShadowMap");"""
    text = text.replace('gEffectModeLocation = glGetUniformLocation(gShaderProgram, "uEffectMode");', bindings)

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)

