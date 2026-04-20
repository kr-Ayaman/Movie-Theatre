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
    float intensity = 20.0; // Global brightness scalar for the grid
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

    if (gEffectModeLocation >= 0) {
        glUseProgram(gShaderProgram);
        glUniform1i(gEffectModeLocation, kSceneShaderEffectDefault);
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