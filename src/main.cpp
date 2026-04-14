#include <GL/glut.h>

#include <cmath>
#include <cctype>
#include <cstdlib>

#include "core/camera.h"
#include "render/lighting.h"
#include "render/shader.h"
#include "scene/room.h"
#include "scene/seats.h"
#include "scene/stage.h"

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

float smoothApproach(float current, float target, float response, float dt) {
    float t = 1.0f - std::exp(-response * dt);
    return current + (target - current) * t;
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

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float eyeX, eyeY, eyeZ;
    float centerX, centerY, centerZ;
    float upX, upY, upZ;
    gCamera.computeLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    gluLookAt(
        eyeX, eyeY, eyeZ,
        centerX, centerY, centerZ,
        upX, upY, upZ
    );

    positionLights();
    enableSceneShader();
    setSceneShaderEffect(kSceneShaderEffectDefault);
    
    updateStageVideo();
    
    drawRoom();
    drawStageAndScreen();
    drawSeats();
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

    float ratio = static_cast<float>(w) / static_cast<float>(h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(52.0f, ratio, 0.1f, 180.0f);
    glMatrixMode(GL_MODELVIEW);

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
