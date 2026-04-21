#include "render/shader.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>

#include <cstdio>

namespace {

GLuint gShaderProgram = 0;
GLint gEffectModeLocation = -1;
GLint gLightSpaceMatrixLoc = -1;
GLint gShadowMapLoc = -1;
GLint gInverseViewMatrixLoc = -1;
GLint gShadowLightDirLoc = -1;
GLint gShadowLightPosLoc = -1;
GLint gShadowLightAmbientLoc = -1;
GLint gShadowLightDiffuseLoc = -1;
GLint gShadowLightSpecularLoc = -1;
GLint gShadowLightEnabledLoc = -1;
GLint gShadowLightModeLoc = -1;
GLint gShadowLightSpotCutoffLoc = -1;
GLint gParallelProjectionLoc = -1;

const char* kVertexShaderSource = R"(
#version 120

varying vec3 vNormal;
varying vec3 vEyePos;
varying vec4 vColor;
varying vec3 vObjectPos;
varying vec3 vObjectNormal;
varying vec4 vLightSpacePos;
varying vec3 vWorldPos;
varying vec3 vWorldNormal;
varying vec3 vWorldEyePos;

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
    
    vWorldPos = worldPos.xyz;
    vWorldEyePos = (uInverseViewMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    vWorldNormal = normalize(mat3(uInverseViewMatrix[0].xyz, uInverseViewMatrix[1].xyz, uInverseViewMatrix[2].xyz) * vNormal);
    
    gl_Position = ftransform();
}
)";

const char* kFragmentShaderSource = R"(
#version 120

varying vec3 vNormal;
varying vec3 vEyePos;
varying vec4 vColor;
varying vec3 vObjectPos;
varying vec3 vObjectNormal;
varying vec4 vLightSpacePos;
varying vec3 vWorldPos;
varying vec3 vWorldNormal;
varying vec3 vWorldEyePos;

uniform int uEffectMode;
uniform sampler2D uShadowMap;
uniform mat4 uInverseViewMatrix;
uniform vec3 uShadowLightDirWorld;
uniform vec3 uShadowLightPosWorld;
uniform vec4 uShadowLightAmbient;
uniform vec4 uShadowLightDiffuse;
uniform vec4 uShadowLightSpecular;
uniform int uShadowLightEnabled;
uniform int uShadowLightMode;
uniform float uShadowLightCosCutoff;
uniform int uParallelProjection;

float computeSpotFactor(int lightIndex, vec3 L) {
    vec3 spotDir = normalize(gl_LightSource[lightIndex].spotDirection);
    float spotCos = dot(-L, spotDir);
    if (spotCos < gl_LightSource[lightIndex].spotCosCutoff) {
        return 0.0;
    }
    return pow(max(spotCos, 0.0), gl_LightSource[lightIndex].spotExponent);
}

vec3 getShadowLightVectorWorld(vec3 worldPos) {
    if (uShadowLightMode == 2) {
        return normalize(uShadowLightPosWorld - worldPos);
    }
    return normalize(uShadowLightDirWorld);
}

float getShadowLightSpotMask(vec3 lightVectorWorld) {
    if (uShadowLightMode != 2) {
        return 1.0;
    }

    float spotCos = dot(-normalize(lightVectorWorld), normalize(uShadowLightDirWorld));
    return smoothstep(uShadowLightCosCutoff - 0.05, min(1.0, uShadowLightCosCutoff + 0.01), spotCos);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 worldNormal, vec3 lightDirWorld) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float ndotl = max(dot(normalize(worldNormal), normalize(lightDirWorld)), 0.0);
    // Use higher bias for seats (3) to prevent acne, and tighter bias for walls/speakers to avoid "Peter Panning"
    float bias = 0.0002;
    
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(2048.0, 2048.0);
    float filterRadius = (uParallelProjection != 0) ? 1.75 : 1.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture2D(uShadowMap, projCoords.xy + vec2(x, y) * texelSize * filterRadius).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    return shadow;
}

void accumulateShadowLight(
    vec3 worldN,
    vec3 worldV,
    inout vec4 ambientAcc,
    inout vec4 diffuseAcc,
    inout vec4 specularAcc,
    float shadowFactor
) {
    if (uShadowLightEnabled == 0 || uShadowLightMode != 1) {
        return;
    }

    vec3 L = normalize(uShadowLightDirWorld);
    // Strong shadow: full visibility reduction for the key directional light
    float visibility = 1.0 - shadowFactor;
    // Ambient is dimmed but never goes fully black — keep some fill in shadows
    float ambientVisibility = 1.0 - shadowFactor * 0.20;
    float ndotl = max(dot(worldN, L), 0.0);

    ambientAcc += uShadowLightAmbient * gl_FrontMaterial.ambient * ambientVisibility;
    if (ndotl <= 0.0) {
        return;
    }

    diffuseAcc += uShadowLightDiffuse * gl_FrontMaterial.diffuse * ndotl * visibility;

    vec3 R = reflect(-L, worldN);
    float specPower = pow(max(dot(R, worldV), 0.0), gl_FrontMaterial.shininess);
    specularAcc += uShadowLightSpecular * gl_FrontMaterial.specular * specPower * visibility;
}

void accumulateLight(
    int lightIndex,
    vec3 N,
    vec3 V,
    vec3 worldN,
    vec3 worldV,
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
        
        // Weaken screen and key lights for seats to maintain top-down lighting balance
        if (uEffectMode == 3 && (lightIndex == 0 || lightIndex == 1)) {
            ndotl *= 0.40;
        }
        // Use the per-call shadowFactor; caller passes 0.0 for unshadowed lights
        float visibility = 1.0 - shadowFactor;
        float ambientVisibility = 1.0 - shadowFactor * 0.20;

        ambientAcc += gl_LightSource[lightIndex].ambient * gl_FrontMaterial.ambient * attenuation * spotFactor * ambientVisibility;
        diffuseAcc += gl_LightSource[lightIndex].diffuse * gl_FrontMaterial.diffuse * ndotl * attenuation * spotFactor * visibility;

        if (ndotl > 0.0) {
            vec3 R = reflect(-L, N);
            float specPower = pow(max(dot(R, V), 0.0), gl_FrontMaterial.shininess);
            specularAcc += gl_LightSource[lightIndex].specular * gl_FrontMaterial.specular * specPower * attenuation * spotFactor * visibility;
        }
        return;
    }

    // --- 77 CEILING LIGHTS --- (Calculated for lightIndex 0 only)
    if (length(gl_LightSource[0].diffuse.rgb) < 0.01) {
        return; // Ceiling lights are off
    }

    // Grid boundaries
    float xMin = -15.2;
    float xMax = 15.2;
    float zMin = -20.6;
    float zMax = 20.6;
    float y = 20.72;

    vec3 gridDiffuse = vec3(0.0);
    vec3 gridSpecular = vec3(0.0);

    // Apply shadow map — moderate strength for room, but weak for seats (3) and gate (5) to avoid harsh self-shadowing
    // Apply shadow map — moderate strength for room, but very weak for seats (3) to avoid self-shadowing
    float shadowMult = 1.0 - shadowFactor * (uEffectMode == 3 ? 0.05 : (uEffectMode == 5 ? 0.45 : 0.45));

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
            }
        }
    }

    // Scale down the sum and multiply by material properties
    float intensity = 18.0; // Leave room for the shadow-casting key light
    vec4 baseDiffuse = gl_LightSource[0].diffuse * gl_FrontMaterial.diffuse;
    vec4 baseSpecular = gl_LightSource[0].specular * gl_FrontMaterial.specular;

    ambientAcc += gl_LightSource[0].ambient * gl_FrontMaterial.ambient * 1.5;
    diffuseAcc += vec4(gridDiffuse * (intensity / 77.0), 0.0) * baseDiffuse;
    specularAcc += vec4(gridSpecular * (intensity / 77.0), 0.0) * baseSpecular;
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
    vec3 worldN = normalize(vWorldNormal);
    if (!gl_FrontFacing) {
        N = -N;
        worldN = -worldN;
    }
    vec3 V = normalize(-vEyePos);
    vec3 worldV = normalize(vWorldEyePos - vWorldPos);

    float shadow = 0.0;
    if (uShadowLightEnabled != 0 && uShadowLightMode != 0) {
        vec3 shadowLightVectorWorld = getShadowLightVectorWorld(vWorldPos);
        float spotMask = getShadowLightSpotMask(shadowLightVectorWorld);
        if (spotMask > 0.001) {
            shadow = calculateShadow(vLightSpacePos, worldN, shadowLightVectorWorld) * spotMask;
        }
    }
    if (uParallelProjection != 0 && uEffectMode == 3) {
        shadow *= 0.35;
    }

    // In screen-light mode (lights off), apply shadow as a global darkness multiplier.
    // This makes seat/curtain silhouettes visible on back wall and side walls even
    // though GL_LIGHT1 (screen) barely illuminates those surfaces.
    float screenShadowGlobalMult = 1.0;
    if (uShadowLightEnabled != 0 && uShadowLightMode == 2) {
        screenShadowGlobalMult = 1.0 - shadow * 1.0;
    }

    vec4 ambientAcc = gl_LightModel.ambient * gl_FrontMaterial.ambient * screenShadowGlobalMult;
    vec4 diffuseAcc = vec4(0.0);
    vec4 specularAcc = vec4(0.0);

    accumulateShadowLight(worldN, worldV, ambientAcc, diffuseAcc, specularAcc, shadow);
    accumulateLight(0, N, V, worldN, worldV, ambientAcc, diffuseAcc, specularAcc, shadow);
    // GL_LIGHT1 = screen spotlight, fully shadowed in screen mode
    accumulateLight(1, N, V, worldN, worldV, ambientAcc, diffuseAcc, specularAcc, shadow);
    // Fill lights also dimmed by screen shadow to deepen the effect
    accumulateLight(2, N, V, worldN, worldV, ambientAcc, diffuseAcc, specularAcc, (uShadowLightMode == 2 ? shadow : 0.0));
    accumulateLight(3, N, V, worldN, worldV, ambientAcc, diffuseAcc, specularAcc, (uShadowLightMode == 2 ? shadow : 0.0));

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
    } else if (uEffectMode == 5) {
        vec3 absObjN = abs(normalize(vObjectNormal));
        vec2 uv;
        if (absObjN.x > absObjN.z && absObjN.x > absObjN.y) {
            uv = vObjectPos.zy;
        } else if (absObjN.z > absObjN.y) {
            uv = vObjectPos.xy;
        } else {
            uv = vObjectPos.xz;
        }

        vec2 panelUv = uv * vec2(1.15, 1.75);
        vec2 panelCell = fract(panelUv + 0.5);
        vec2 panelId = floor(panelUv + 0.5);
        float edgeDistance = min(min(panelCell.x, 1.0 - panelCell.x), min(panelCell.y, 1.0 - panelCell.y));
        float seamMask = 1.0 - smoothstep(0.035, 0.085, edgeDistance);

        float panelNoise = fract(sin(dot(panelId, vec2(31.173, 47.551))) * 17853.127);
        float verticalBrush = 0.5 + 0.5 * sin(uv.y * 120.0 + sin(uv.x * 7.0) * 2.2);
        float crossBrush = 0.5 + 0.5 * sin(uv.x * 52.0 - uv.y * 17.0);
        float grain = mix(verticalBrush, crossBrush, absObjN.y * 0.35);

        float bottomWear = clamp(0.55 - vObjectPos.y * 0.12, 0.0, 1.0);
        float rim = pow(clamp(1.0 - max(dot(N, V), 0.0), 0.0, 1.0), 2.4);

        shadedRgb *= 0.90 + (grain - 0.5) * 0.18;
        shadedRgb *= 0.94 + panelNoise * 0.10;
        shadedRgb *= vec3(0.95, 0.97, 1.02);
        shadedRgb *= 1.0 - bottomWear * 0.08;

        vec3 seamColor = shadedRgb * vec3(0.42, 0.44, 0.50);
        shadedRgb = mix(shadedRgb, seamColor, seamMask * 0.55);
        shadedRgb += vec3(0.15, 0.17, 0.20) * rim * 0.16;
    } else if (uEffectMode == 6) {
        vec3 absObjN = abs(normalize(vObjectNormal));
        vec2 uv;
        if (absObjN.x > absObjN.z && absObjN.x > absObjN.y) {
            uv = vObjectPos.zy;
        } else if (absObjN.z > absObjN.y) {
            uv = vObjectPos.xy;
        } else {
            uv = vObjectPos.xz;
        }

        float brushA = 0.5 + 0.5 * sin(uv.x * 190.0 + sin(uv.y * 9.0) * 1.8);
        float brushB = 0.5 + 0.5 * sin(uv.x * 330.0 + uv.y * 11.0);
        float brushed = mix(brushA, brushB, 0.45);
        float softBand = 0.5 + 0.5 * cos(uv.y * 18.0);
        float rim = pow(clamp(1.0 - max(dot(N, V), 0.0), 0.0, 1.0), 2.0);

        shadedRgb *= vec3(0.94, 0.97, 1.05);
        shadedRgb *= 0.97 + (brushed - 0.5) * 0.10;
        shadedRgb += vec3(0.10, 0.12, 0.16) * softBand * 0.10;
        shadedRgb += vec3(0.20, 0.22, 0.28) * rim * 0.12;
    }

    gl_FragColor = vec4(clamp(shadedRgb, 0.0, 1.0), litColor.a);
}
)";

void logShaderInfo(GLuint shader, const char* label) {
    GLint logLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength <= 1) {
        return;
    }

    char buffer[2048];
    GLsizei written = 0;
    glGetShaderInfoLog(shader, sizeof(buffer), &written, buffer);
    if (written > 0) {
        std::fprintf(stderr, "%s: %s\n", label, buffer);
    }
}

void logProgramInfo(GLuint program, const char* label) {
    GLint logLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength <= 1) {
        return;
    }

    char buffer[2048];
    GLsizei written = 0;
    glGetProgramInfoLog(program, sizeof(buffer), &written, buffer);
    if (written > 0) {
        std::fprintf(stderr, "%s: %s\n", label, buffer);
    }
}

GLuint compileShader(GLenum type, const char* source, const char* label) {
    GLuint shader = glCreateShader(type);
    if (shader == 0) {
        std::fprintf(stderr, "Failed to create %s.\n", label);
        return 0;
    }

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint compileOk = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileOk);
    logShaderInfo(shader, label);
    if (compileOk != GL_TRUE) {
        std::fprintf(stderr, "Failed to compile %s.\n", label);
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

}  // namespace

bool initSceneShader() {
    if (gShaderProgram != 0) {
        return true;
    }

    if (glGetString(GL_SHADING_LANGUAGE_VERSION) == nullptr) {
        std::fprintf(stderr, "GLSL is unavailable; continuing without custom shaders.\n");
        return false;
    }

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, kVertexShaderSource, "Vertex shader");
    if (vertexShader == 0) {
        return false;
    }

    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, kFragmentShaderSource, "Fragment shader");
    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        return false;
    }

    GLuint program = glCreateProgram();
    if (program == 0) {
        std::fprintf(stderr, "Failed to create shader program.\n");
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint linkOk = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkOk);
    logProgramInfo(program, "Shader program");

    glDetachShader(program, vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if (linkOk != GL_TRUE) {
        std::fprintf(stderr, "Failed to link shader program.\n");
        glDeleteProgram(program);
        return false;
    }

    gShaderProgram = program;
    gEffectModeLocation = glGetUniformLocation(gShaderProgram, "uEffectMode");
    gLightSpaceMatrixLoc = glGetUniformLocation(gShaderProgram, "uLightSpaceMatrix");
    gInverseViewMatrixLoc = glGetUniformLocation(gShaderProgram, "uInverseViewMatrix");
    gShadowMapLoc = glGetUniformLocation(gShaderProgram, "uShadowMap");
    gShadowLightDirLoc = glGetUniformLocation(gShaderProgram, "uShadowLightDirWorld");
    gShadowLightPosLoc = glGetUniformLocation(gShaderProgram, "uShadowLightPosWorld");
    gShadowLightAmbientLoc = glGetUniformLocation(gShaderProgram, "uShadowLightAmbient");
    gShadowLightDiffuseLoc = glGetUniformLocation(gShaderProgram, "uShadowLightDiffuse");
    gShadowLightSpecularLoc = glGetUniformLocation(gShaderProgram, "uShadowLightSpecular");
    gShadowLightEnabledLoc = glGetUniformLocation(gShaderProgram, "uShadowLightEnabled");
    gShadowLightModeLoc = glGetUniformLocation(gShaderProgram, "uShadowLightMode");
    gShadowLightSpotCutoffLoc = glGetUniformLocation(gShaderProgram, "uShadowLightCosCutoff");
    gParallelProjectionLoc = glGetUniformLocation(gShaderProgram, "uParallelProjection");

    if (gEffectModeLocation >= 0) {
        glUseProgram(gShaderProgram);
        glUniform1i(gEffectModeLocation, kSceneShaderEffectDefault);
        if (gShadowLightEnabledLoc >= 0) {
            glUniform1i(gShadowLightEnabledLoc, 0);
        }
        if (gShadowLightModeLoc >= 0) {
            glUniform1i(gShadowLightModeLoc, kShadowLightModeNone);
        }
        if (gParallelProjectionLoc >= 0) {
            glUniform1i(gParallelProjectionLoc, 0);
        }
        glUseProgram(0);
    }

    return true;
}

void enableSceneShader() {
    if (gShaderProgram != 0) {
        glUseProgram(gShaderProgram);
    }
}

void disableSceneShader() {
    glUseProgram(0);
}


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

void setShadowLightDirection(const float direction[3]) {
    if (gShadowLightDirLoc != -1) {
        glUniform3fv(gShadowLightDirLoc, 1, direction);
    }
}

void setShadowLightPosition(const float position[3]) {
    if (gShadowLightPosLoc != -1) {
        glUniform3fv(gShadowLightPosLoc, 1, position);
    }
}

void setShadowLightAmbient(const float color[4]) {
    if (gShadowLightAmbientLoc != -1) {
        glUniform4fv(gShadowLightAmbientLoc, 1, color);
    }
}

void setShadowLightDiffuse(const float color[4]) {
    if (gShadowLightDiffuseLoc != -1) {
        glUniform4fv(gShadowLightDiffuseLoc, 1, color);
    }
}

void setShadowLightSpecular(const float color[4]) {
    if (gShadowLightSpecularLoc != -1) {
        glUniform4fv(gShadowLightSpecularLoc, 1, color);
    }
}

void setShadowLightMode(int mode) {
    if (gShadowLightModeLoc != -1) {
        glUniform1i(gShadowLightModeLoc, mode);
    }
}

void setShadowLightSpotCutoff(float cosCutoff) {
    if (gShadowLightSpotCutoffLoc != -1) {
        glUniform1f(gShadowLightSpotCutoffLoc, cosCutoff);
    }
}

void setShadowLightEnabled(bool enabled) {
    if (gShadowLightEnabledLoc != -1) {
        glUniform1i(gShadowLightEnabledLoc, enabled ? 1 : 0);
    }
}

void setParallelProjectionEnabled(bool enabled) {
    if (gParallelProjectionLoc != -1) {
        glUniform1i(gParallelProjectionLoc, enabled ? 1 : 0);
    }
}

void setSceneShaderEffect(int effectMode) {
    if (gShaderProgram == 0 || gEffectModeLocation < 0) {
        return;
    }
    glUniform1i(gEffectModeLocation, effectMode);
}

bool isSceneShaderActive() {
    return gShaderProgram != 0;
}

void shutdownSceneShader() {
    if (gShaderProgram != 0) {
        glDeleteProgram(gShaderProgram);
        gShaderProgram = 0;
        gEffectModeLocation = -1;
    }
}
