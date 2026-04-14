#include "render/shader.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>

#include <cstdio>

namespace {

GLuint gShaderProgram = 0;
GLint gEffectModeLocation = -1;

const char* kVertexShaderSource = R"(
#version 120

varying vec3 vNormal;
varying vec3 vEyePos;
varying vec4 vColor;
varying vec3 vObjectPos;
varying vec3 vObjectNormal;

void main() {
    vec4 eyePos = gl_ModelViewMatrix * gl_Vertex;
    vEyePos = eyePos.xyz;
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    vColor = gl_Color;
    vObjectPos = gl_Vertex.xyz;
    vObjectNormal = normalize(gl_Normal);
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

uniform int uEffectMode;

float computeSpotFactor(int lightIndex, vec3 L) {
    vec3 spotDir = normalize(gl_LightSource[lightIndex].spotDirection);
    float spotCos = dot(-L, spotDir);
    if (spotCos < gl_LightSource[lightIndex].spotCosCutoff) {
        return 0.0;
    }
    return pow(max(spotCos, 0.0), gl_LightSource[lightIndex].spotExponent);
}

void accumulateLight(
    int lightIndex,
    vec3 N,
    vec3 V,
    inout vec4 ambientAcc,
    inout vec4 diffuseAcc,
    inout vec4 specularAcc
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
    ambientAcc += gl_LightSource[lightIndex].ambient * gl_FrontMaterial.ambient * attenuation * spotFactor;
    diffuseAcc += gl_LightSource[lightIndex].diffuse * gl_FrontMaterial.diffuse * ndotl * attenuation * spotFactor;

    if (ndotl > 0.0) {
        vec3 R = reflect(-L, N);
        float specPower = pow(max(dot(R, V), 0.0), gl_FrontMaterial.shininess);
        specularAcc += gl_LightSource[lightIndex].specular * gl_FrontMaterial.specular * specPower * attenuation * spotFactor;
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

    vec4 ambientAcc = gl_LightModel.ambient * gl_FrontMaterial.ambient;
    vec4 diffuseAcc = vec4(0.0);
    vec4 specularAcc = vec4(0.0);

    accumulateLight(0, N, V, ambientAcc, diffuseAcc, specularAcc);
    accumulateLight(1, N, V, ambientAcc, diffuseAcc, specularAcc);
    accumulateLight(2, N, V, ambientAcc, diffuseAcc, specularAcc);
    accumulateLight(3, N, V, ambientAcc, diffuseAcc, specularAcc);

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