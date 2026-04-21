#include "scene/stage.h"

#include "render/lighting.h"
#include "render/primitives.h"
#include "render/shader.h"
#include "render/video.h"

#include <GL/glut.h>

#include <cmath>
#include <memory>

namespace {
const float kPi = 3.14159265f;

std::unique_ptr<VideoPlayer> gVideoPlayer = nullptr;

void drawScreenContent() {
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 9.2f, -19.04f);

    // When ceiling lights are on, the screen should remain black.
    if (areCeilingLightsEnabled()) {
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_QUADS);
        glVertex3f(-11.0f, -4.7f, 0.0f);
        glVertex3f(11.0f, -4.7f, 0.0f);
        glVertex3f(11.0f, 4.7f, 0.0f);
        glVertex3f(-11.0f, 4.7f, 0.0f);
        glEnd();
    } else if (gVideoPlayer && gVideoPlayer->isLoaded()) {
        disableSceneShader();

        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glBindTexture(GL_TEXTURE_2D, gVideoPlayer->getTextureId());

        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-11.0f, -4.7f, 0.0f);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(11.0f, -4.7f, 0.0f);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(11.0f, 4.7f, 0.0f);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-11.0f, 4.7f, 0.0f);
        glEnd();

        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);

        enableSceneShader();
    } else {
        // Default content with animated circles
        glBegin(GL_QUADS);
        glColor3f(0.03f, 0.06f, 0.13f);
        glVertex3f(-11.0f, -4.7f, 0.0f);
        glColor3f(0.07f, 0.16f, 0.32f);
        glVertex3f(11.0f, -4.7f, 0.0f);
        glColor3f(0.16f, 0.28f, 0.55f);
        glVertex3f(11.0f, 4.7f, 0.0f);
        glColor3f(0.07f, 0.12f, 0.25f);
        glVertex3f(-11.0f, 4.7f, 0.0f);
        glEnd();

        drawSoftCircle(-4.5f, 1.6f, 3.3f, 0.38f, 0.20f, 0.74f, 0.42f);
        drawSoftCircle(2.8f, 0.8f, 3.8f, 0.10f, 0.58f, 0.86f, 0.40f);
        drawSoftCircle(1.3f, -1.7f, 2.9f, 0.66f, 0.14f, 0.70f, 0.36f);
        drawSoftCircle(-1.4f, 2.8f, 2.3f, 0.20f, 0.80f, 0.98f, 0.30f);

        glColor4f(0.03f, 0.03f, 0.05f, 0.94f);
        glBegin(GL_POLYGON);
        glVertex3f(-0.4f, -0.8f, 0.01f);
        glVertex3f(0.0f, 0.2f, 0.01f);
        glVertex3f(0.7f, 1.3f, 0.01f);
        glVertex3f(1.4f, 1.8f, 0.01f);
        glVertex3f(2.0f, 1.2f, 0.01f);
        glVertex3f(1.7f, 0.3f, 0.01f);
        glVertex3f(1.0f, -0.5f, 0.01f);
        glVertex3f(0.2f, -1.0f, 0.01f);
        glEnd();

        glColor4f(0.03f, 0.03f, 0.05f, 0.94f);
        glBegin(GL_POLYGON);
        glVertex3f(-2.8f, -0.9f, 0.01f);
        glVertex3f(-2.2f, 0.5f, 0.01f);
        glVertex3f(-1.7f, 1.9f, 0.01f);
        glVertex3f(-1.1f, 1.4f, 0.01f);
        glVertex3f(-1.2f, 0.3f, 0.01f);
        glVertex3f(-1.7f, -0.7f, 0.01f);
        glEnd();

        glColor4f(0.50f, 0.66f, 0.88f, 0.18f);
        glBegin(GL_LINES);
        for (int i = 0; i < 20; ++i) {
            float y = -4.4f + i * 0.45f;
            glVertex3f(-10.8f, y, 0.02f);
            glVertex3f(10.8f, y + 0.08f, 0.02f);
        }
        glEnd();
    }

    glPopMatrix();
    glEnable(GL_LIGHTING);
}

void drawCurtain(bool leftSide) {
    float side = leftSide ? -1.0f : 1.0f;
    float startX = leftSide ? -13.3f : 13.3f;

    setSceneShaderEffect(kSceneShaderEffectCurtain);

    int numFolds = 40;
    float foldWidth = 0.18f;
    for (int fold = 0; fold < numFolds; ++fold) {
        float offset = fold * foldWidth * side;
        // Maintain the same wave shape as before but with higher resolution
        float wavePhase = (fold * 8.1f / numFolds);
        float depthWave = std::sin(wavePhase) * 0.35f;
        float tint = 0.22f + (depthWave + 0.35f) * 0.05f;
        setMaterial(0.07f, tint, 0.52f, 45.0f, 0.30f);
        drawBlock(startX + offset, 8.5f, -10.8f + depthWave, foldWidth + 0.02f, 14.5f, 1.25f);
    }

    setSceneShaderEffect(kSceneShaderEffectDefault);
}

void drawBeanBag(float x, float z) {
    setMaterial(0.18f, 0.34f, 0.58f, 28.0f, 0.24f);
    glPushMatrix();
    glTranslatef(x, 1.35f, z);
    glScalef(1.4f, 0.56f, 1.1f);
    glutSolidSphere(1.0, 26, 20);
    glPopMatrix();
}

}  // namespace

void drawStageAndScreen() {
    setSceneShaderEffect(kSceneShaderEffectDefault);

    setSceneShaderEffect(kSceneShaderEffectBrick);
    setMaterial(0.70f, 0.72f, 0.74f, 16.0f, 0.08f);
    drawBlock(0.0f, 1.0f, -14.0f, 28.0f, 2.0f, 12.0f);
    drawBlock(0.0f, 0.3f, -7.4f, 23.0f, 0.6f, 2.6f);

    setMaterial(0.66f, 0.68f, 0.70f, 14.0f, 0.07f);
    drawBlock(-14.2f, 8.6f, -11.6f, 1.2f, 15.2f, 1.5f);
    drawBlock(14.2f, 8.6f, -11.6f, 1.2f, 15.2f, 1.5f);
    drawBlock(0.0f, 15.5f, -11.6f, 30.0f, 1.3f, 1.5f);

    setSceneShaderEffect(kSceneShaderEffectDefault);

    setMaterial(0.08f, 0.09f, 0.10f, 15.0f, 0.16f);
    drawBlock(0.0f, 9.2f, -19.6f, 24.7f, 11.6f, 0.90f);

    setMaterial(0.14f, 0.16f, 0.21f, 22.0f, 0.24f, 0.06f);
    drawBlock(0.0f, 9.2f, -19.2f, 22.6f, 9.8f, 0.25f);
    drawScreenContent();

    drawCurtain(true);
    drawCurtain(false);

    drawBeanBag(-1.8f, -8.2f);
    drawBeanBag(1.8f, -8.3f);
}

void initStageVideo(const std::string& videoPath) {
    if (!gVideoPlayer) {
        gVideoPlayer = std::make_unique<VideoPlayer>();
    }
    gVideoPlayer->loadVideo(videoPath);
}

void updateStageVideo() {
    if (gVideoPlayer && gVideoPlayer->isLoaded()) {
        gVideoPlayer->updateFrame();
    }
}
