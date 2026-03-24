#include <GL/glut.h>

#include <cstdlib>

#include "core/camera.h"
#include "render/lighting.h"
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

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float camX, camY, camZ;
    gCamera.computePosition(camX, camY, camZ);

    gluLookAt(
        camX, camY, camZ,
        gCamera.targetX, gCamera.targetY, gCamera.targetZ,
        0.0f, 1.0f, 0.0f
    );

    positionLights();
    drawRoom();
    drawStageAndScreen();
    drawSeats();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) {
        h = 1;
    }

    float ratio = static_cast<float>(w) / static_cast<float>(h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(52.0f, ratio, 0.1f, 180.0f);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int, int) {
    switch (key) {
        case 'w':
            gCamera.distance -= 1.0f;
            break;
        case 's':
            gCamera.distance += 1.0f;
            break;
        case 'a':
            gCamera.yaw -= 3.0f;
            break;
        case 'd':
            gCamera.yaw += 3.0f;
            break;
        case 'q':
            gCamera.targetY += 0.5f;
            break;
        case 'e':
            gCamera.targetY -= 0.5f;
            break;
        case 'r':
            gCamera.reset();
            break;
        case 27:
            std::exit(0);
            break;
        default:
            break;
    }

    gCamera.clamp();
    glutPostRedisplay();
}

void specialKeys(int key, int, int) {
    switch (key) {
        case GLUT_KEY_LEFT:
            gCamera.yaw -= 2.5f;
            break;
        case GLUT_KEY_RIGHT:
            gCamera.yaw += 2.5f;
            break;
        case GLUT_KEY_UP:
            gCamera.pitch += 2.0f;
            break;
        case GLUT_KEY_DOWN:
            gCamera.pitch -= 2.0f;
            break;
        default:
            break;
    }

    gCamera.clamp();
    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        gLeftMouseDown = (state == GLUT_DOWN);
    } else if (button == GLUT_RIGHT_BUTTON) {
        gRightMouseDown = (state == GLUT_DOWN);
    } else if (button == GLUT_MIDDLE_BUTTON) {
        gMiddleMouseDown = (state == GLUT_DOWN);
    } else if (button == 3 && state == GLUT_DOWN) {
        gCamera.distance -= 1.0f;
        gCamera.clamp();
        glutPostRedisplay();
    } else if (button == 4 && state == GLUT_DOWN) {
        gCamera.distance += 1.0f;
        gCamera.clamp();
        glutPostRedisplay();
    }

    gLastMouseX = x;
    gLastMouseY = y;
}

void mouseMotion(int x, int y) {
    int dx = x - gLastMouseX;
    int dy = y - gLastMouseY;

    if (gLeftMouseDown) {
        gCamera.yaw += dx * 0.35f;
        gCamera.pitch -= dy * 0.25f;
    }

    if (gRightMouseDown) {
        gCamera.pan(dx, dy);
    }

    if (gMiddleMouseDown) {
        gCamera.distance += dy * 0.12f;
    }

    gCamera.clamp();
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
}

}  // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1100, 700);
    glutCreateWindow("OpenGL Theatre - Modular Scene");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);

    glutMainLoop();
    return 0;
}
