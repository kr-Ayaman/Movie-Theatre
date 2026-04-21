#define GL_GLEXT_PROTOTYPES
#include <GL/glut.h>
#include <GL/glext.h>

#include <cmath>
#include <cctype>
#include <cstdlib>

#include "core/camera.h"
#include "render/lighting.h"
#include "render/shader.h"
#include "scene/room.h"
#include "scene/seats.h"
#include "scene/stage.h"
#include "core/math_utils.h"

namespace {
Camera gCamera;
bool gLeftMouseDown = false;
bool gRightMouseDown = false;
bool gMiddleMouseDown = false;
int gLastMouseX = 0;
int gLastMouseY = 0;

bool gKeyDown[256] = {};
bool gSpecialDown[256] = {};

float gForwardVel = 0.0f;
float gRightVel = 0.0f;
float gUpVel = 0.0f;
float gYawVel = 0.0f;
float gPitchVel = 0.0f;

int gLastFrameTimeMs = 0;

GLuint gDepthMapFBO;
GLuint gDepthMap;
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
float gLightSpaceMatrix[16];
float gInverseViewMatrix[16];

void loadLightViewMatrix(const float eye[3], const float dir[3]) {
    glLoadIdentity();
    float target[3] = {eye[0] + dir[0], eye[1] + dir[1], eye[2] + dir[2]};
    float up[3] = {0.0f, 1.0f, 0.0f};
    if (std::abs(dir[1]) > 0.99f) {
        up[0] = 0.0f; up[1] = 0.0f; up[2] = -1.0f;
    }
    gluLookAt(eye[0], eye[1], eye[2], target[0], target[1], target[2], up[0], up[1], up[2]);
}

void multiplyProjectionAndView(float proj[16], float view[16]) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixf(proj);
    glMultMatrixf(view);
    glGetFloatv(GL_PROJECTION_MATRIX, gLightSpaceMatrix);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void updateDirectionalLightSpaceMatrix(const float shadowLightDir[3]) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-34.0f, 34.0f, -48.0f, 48.0f, 1.0f, 120.0f);
    float lightProjMat[16];
    glGetFloatv(GL_PROJECTION_MATRIX, lightProjMat);

    glMatrixMode(GL_MODELVIEW);
    float lightEye[3] = {shadowLightDir[0] * 68.0f, shadowLightDir[1] * 68.0f, shadowLightDir[2] * 68.0f};
    float viewDir[3] = {-shadowLightDir[0], -shadowLightDir[1], -shadowLightDir[2]};
    loadLightViewMatrix(lightEye, viewDir);
    float lightViewMat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMat);

    multiplyProjectionAndView(lightProjMat, lightViewMat);
    glMatrixMode(GL_MODELVIEW);
}

void updateSpotLightSpaceMatrix(const float shadowLightPos[3], const float shadowLightDir[3]) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(100.0f, 1.0f, 0.5f, 100.0f);
    float lightProjMat[16];
    glGetFloatv(GL_PROJECTION_MATRIX, lightProjMat);

    glMatrixMode(GL_MODELVIEW);
    loadLightViewMatrix(shadowLightPos, shadowLightDir);
    float lightViewMat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMat);

    multiplyProjectionAndView(lightProjMat, lightViewMat);
    glMatrixMode(GL_MODELVIEW);
}

float smoothApproach(float current, float target, float response, float dt) {
    float t = 1.0f - std::exp(-response * dt);
    return current + (target - current) * t;
}

void setupCameraProjection(int w, int h) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float ratio = static_cast<float>(w) / static_cast<float>(h);
    
    if (gCamera.projectionMode == PROJECTION_ORTHOGRAPHIC) {
        // Orthographic (parallel) projection
        float orthoHeight = gCamera.orthoScale;
        float orthoWidth = orthoHeight * ratio;
        glOrtho(-orthoWidth / 2.0f, orthoWidth / 2.0f, 
                -orthoHeight / 2.0f, orthoHeight / 2.0f, 
                0.1f, 300.0f);
    } else {
        // Perspective projection (default)
        gluPerspective(52.0f, ratio, 0.1f, 180.0f);
    }
    
    glMatrixMode(GL_MODELVIEW);
}

bool isAsciiKeyDown(char key) {
    unsigned char lower = static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(key)));
    unsigned char upper = static_cast<unsigned char>(std::toupper(static_cast<unsigned char>(key)));
    return gKeyDown[lower] || gKeyDown[upper];
}

void clearMotion() {
    gForwardVel = 0.0f;
    gRightVel = 0.0f;
    gUpVel = 0.0f;
    gYawVel = 0.0f;
    gPitchVel = 0.0f;
}

void idle() {
    int nowMs = glutGet(GLUT_ELAPSED_TIME);
    if (gLastFrameTimeMs == 0) {
        gLastFrameTimeMs = nowMs;
        return;
    }

    float dt = (nowMs - gLastFrameTimeMs) * 0.001f;
    gLastFrameTimeMs = nowMs;
    if (dt <= 0.0f) {
        return;
    }
    if (dt > 0.05f) {
        dt = 0.05f;
    }

    float forwardIntent = (isAsciiKeyDown('w') ? 1.0f : 0.0f) - (isAsciiKeyDown('s') ? 1.0f : 0.0f);
    float rightIntent = (isAsciiKeyDown('d') ? 1.0f : 0.0f) - (isAsciiKeyDown('a') ? 1.0f : 0.0f);
    float upIntent = ((isAsciiKeyDown('q') || gKeyDown[static_cast<unsigned char>(' ')]) ? 1.0f : 0.0f) -
                     ((isAsciiKeyDown('e') || isAsciiKeyDown('c')) ? 1.0f : 0.0f);

    if (gSpecialDown[GLUT_KEY_PAGE_UP]) {
        upIntent += 1.0f;
    }
    if (gSpecialDown[GLUT_KEY_PAGE_DOWN]) {
        upIntent -= 1.0f;
    }
    if (upIntent > 1.0f) {
        upIntent = 1.0f;
    }
    if (upIntent < -1.0f) {
        upIntent = -1.0f;
    }

    float yawIntent = (gSpecialDown[GLUT_KEY_RIGHT] ? 1.0f : 0.0f) - (gSpecialDown[GLUT_KEY_LEFT] ? 1.0f : 0.0f);
    float pitchIntent = (gSpecialDown[GLUT_KEY_UP] ? 1.0f : 0.0f) - (gSpecialDown[GLUT_KEY_DOWN] ? 1.0f : 0.0f);

    const float moveSpeed = 12.0f;
    const float moveResponse = 10.0f;
    const float rotateSpeed = 110.0f;
    const float rotateResponse = 13.0f;

    gForwardVel = smoothApproach(gForwardVel, forwardIntent * moveSpeed, moveResponse, dt);
    gRightVel = smoothApproach(gRightVel, rightIntent * moveSpeed, moveResponse, dt);
    gUpVel = smoothApproach(gUpVel, upIntent * moveSpeed, moveResponse, dt);
    gYawVel = smoothApproach(gYawVel, yawIntent * rotateSpeed, rotateResponse, dt);
    gPitchVel = smoothApproach(gPitchVel, pitchIntent * rotateSpeed, rotateResponse, dt);

    gCamera.moveLocal(gForwardVel * dt, gRightVel * dt, gUpVel * dt);
    gCamera.rotate(gYawVel * dt, gPitchVel * dt);

    updateStageVideo();

    if (std::fabs(gForwardVel) > 0.001f || std::fabs(gRightVel) > 0.001f || std::fabs(gUpVel) > 0.001f ||
        std::fabs(gYawVel) > 0.001f || std::fabs(gPitchVel) > 0.001f) {
        glutPostRedisplay();
    }

    // Redraw continuously while video is playing so the screen updates.
    glutPostRedisplay();
}

void renderScene() {
    drawRoom();
    drawStageAndScreen();
    drawSeats();
}

void display() {
    updateStageVideo();

    float eyeX, eyeY, eyeZ;
    float centerX, centerY, centerZ;
    float upX, upY, upZ;
    gCamera.computeLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    computeInverseViewMatrix(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ, gInverseViewMatrix);

    float shadowLightDir[3];
    getShadowLightDirection(shadowLightDir);
    float shadowLightPos[3];
    getScreenLightPosition(shadowLightPos);
    int shadowLightMode = areCeilingLightsEnabled() ? kShadowLightModeCeiling : kShadowLightModeScreen;

    // 1. Render depth of scene to texture (from light's perspective)
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, gDepthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    if (shadowLightMode == kShadowLightModeCeiling) {
        updateDirectionalLightSpaceMatrix(shadowLightDir);
    } else {
        updateSpotLightSpaceMatrix(shadowLightPos, shadowLightDir);
    }

    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    setShadowPassMode(true);
    renderScene();
    setShadowPassMode(false);

    glDisable(GL_CULL_FACE);
    glDisable(GL_POLYGON_OFFSET_FILL);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2. Render scene as normal
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);
    glViewport(0, 0, w, h);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setupCameraProjection(w, h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    positionLights();
    
    enableSceneShader();
    setSceneShaderEffect(kSceneShaderEffectDefault);
    
    setShadowLightDirection(shadowLightDir);
    setShadowLightPosition(shadowLightPos);
    setShadowLightMode(shadowLightMode);
    setShadowLightEnabled(true);
    setParallelProjectionEnabled(gCamera.projectionMode == PROJECTION_ORTHOGRAPHIC);

    setLightSpaceMatrix(gLightSpaceMatrix);
    setInverseViewMatrix(gInverseViewMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gDepthMap);
    setShadowMap(1);
    glActiveTexture(GL_TEXTURE0);

    renderScene();
    
    disableSceneShader();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (w == 0) {
        w = 1;
    }
    if (h == 0) {
        h = 1;
    }

    glViewport(0, 0, w, h);

    setupCameraProjection(w, h);

    glutPostRedisplay();
}

void keyboardDown(unsigned char key, int, int) {
    gKeyDown[key] = true;

    if (key == 27) {
        std::exit(0);
    }

    unsigned char lower = static_cast<unsigned char>(std::tolower(key));
    if (lower == 'r') {
        gCamera.reset();
        clearMotion();
        glutPostRedisplay();
    } else if (lower == 'z') {
        gCamera.zoom(1.0f);
        glutPostRedisplay();
    } else if (lower == 'x') {
        gCamera.zoom(-1.0f);
        glutPostRedisplay();
    } else if (lower == 'l') {
        toggleCeilingLights();
        setCeilingLightsEnabled(areCeilingLightsVisible());
        glutPostRedisplay();
    } else if (lower == 'p') {
        gCamera.toggleProjectionMode();
        glutPostRedisplay();
    } else if (key == '+' || key == '=') {
        gCamera.adjustOrthoScale(-2.0f);  // Zoom in (decrease ortho scale)
        glutPostRedisplay();
    } else if (key == '-' || key == '_') {
        gCamera.adjustOrthoScale(2.0f);   // Zoom out (increase ortho scale)
        glutPostRedisplay();
    }
}


void keyboardUp(unsigned char key, int, int) {
    gKeyDown[key] = false;
}

void specialDown(int key, int, int) {
    if (key >= 0 && key < 256) {
        gSpecialDown[key] = true;
    }
}

void specialUp(int key, int, int) {
    if (key >= 0 && key < 256) {
        gSpecialDown[key] = false;
    }
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        gLeftMouseDown = (state == GLUT_DOWN);
    } else if (button == GLUT_RIGHT_BUTTON) {
        gRightMouseDown = (state == GLUT_DOWN);
    } else if (button == GLUT_MIDDLE_BUTTON) {
        gMiddleMouseDown = (state == GLUT_DOWN);
    } else if (button == 3 && state == GLUT_DOWN) {
        gCamera.zoom(1.0f);
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) {
        gCamera.zoom(-1.0f);
        glutPostRedisplay();
    }

    gLastMouseX = x;
    gLastMouseY = y;
}

void mouseMotion(int x, int y) {
    int dx = x - gLastMouseX;
    int dy = y - gLastMouseY;

    if (gLeftMouseDown) {
        gCamera.rotate(dx * 0.32f, -dy * 0.24f);
    }

    if (gRightMouseDown) {
        gCamera.pan(dx, dy);
    }

    if (gMiddleMouseDown) {
        gCamera.zoom(-dy * 0.12f);
    }

    gLastMouseX = x;
    gLastMouseY = y;
    glutPostRedisplay();
}

void init() {
    glClearColor(0.06f, 0.06f, 0.08f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    initLighting();
    setCeilingLightsEnabled(areCeilingLightsVisible());
    initSceneShader();

    glGenFramebuffers(1, &gDepthMapFBO);
    glGenTextures(1, &gDepthMap);
    glBindTexture(GL_TEXTURE_2D, gDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, gDepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
        std::printf("Shadow FBO OK\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

}  // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1100, 700);
    glutCreateWindow("OpenGL Theatre - Video Player");

    init();
    gLastFrameTimeMs = glutGet(GLUT_ELAPSED_TIME);

    // Load and initialize video
    initStageVideo("14422313_4096_2160_60fps.mp4");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}
