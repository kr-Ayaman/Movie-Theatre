import re

with open('src/main.cpp', 'r') as f:
    text = f.read()

# Add includes
text = text.replace('#include "scene/stage.h"', '#include "scene/stage.h"\n#include "core/math_utils.h"')

# Add globals
global_vars = r'''int gLastFrameTimeMs = 0;

GLuint gDepthMapFBO;
GLuint gDepthMap;
const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
float gLightSpaceMatrix[16];
float gInverseViewMatrix[16];'''
text = re.sub(r'int gLastFrameTimeMs = 0;', global_vars, text)

# Add init shadowing
init_end_search = r'''    initLighting\(\);
    setCeilingLightsEnabled\(areCeilingLightsVisible\(\)\);
    initSceneShader\(\);
\}'''

init_end_replace = r'''    initLighting();
    setCeilingLightsEnabled(areCeilingLightsVisible());
    initSceneShader();

    glGenFramebuffers(1, &gDepthMapFBO);
    glGenTextures(1, &gDepthMap);
    glBindTexture(GL_TEXTURE_2D, gDepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, gDepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}'''
text = re.sub(init_end_search, init_end_replace, text)

# Update display function
display_search = r'''void display\(\) \{
    glClear\(GL_COLOR_BUFFER_BIT \| GL_DEPTH_BUFFER_BIT\);
    glMatrixMode\(GL_MODELVIEW\);
    glLoadIdentity\(\);

    float eyeX, eyeY, eyeZ;
    float centerX, centerY, centerZ;
    float upX, upY, upZ;
    gCamera.computeLookAt\(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ\);

    gluLookAt\(
        eyeX, eyeY, eyeZ,
        centerX, centerY, centerZ,
        upX, upY, upZ
    \);

    positionLights\(\);
    enableSceneShader\(\);
    setSceneShaderEffect\(kSceneShaderEffectDefault\);
    
    updateStageVideo\(\);
    
    drawRoom\(\);
    drawStageAndScreen\(\);
    drawSeats\(\);
    disableSceneShader\(\);

    glutSwapBuffers\(\);
\}'''

display_replace = r'''void renderScene() {
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

    float lightPos[3] = {0.0f, 20.0f, -8.0f};

    // 1. Render depth of scene to texture (from light's perspective)
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, gDepthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(60.0f, (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT, 2.0f, 80.0f);
    
    float lightProjMat[16];
    glGetFloatv(GL_PROJECTION_MATRIX, lightProjMat);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(lightPos[0], lightPos[1], lightPos[2],
              0.0f, 0.0f, 0.0f,
              0.0f, 1.0f, 0.0f);
              
    float lightViewMat[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, lightViewMat);

    // Compute LightSpaceMatrix = Proj * View
    for (int i=0; i<4; ++i) {
        for (int j=0; j<4; ++j) {
            gLightSpaceMatrix[i*4 + j] = 0;
            for (int k=0; k<4; ++k) {
                gLightSpaceMatrix[i*4 + j] += lightProjMat[k*4 + j] * lightViewMat[i*4 + k];
            }
        }
    }

    renderScene();

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

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(52.0f, (float)w / (float)h, 0.1f, 180.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    positionLights();
    
    enableSceneShader();
    setSceneShaderEffect(kSceneShaderEffectDefault);
    
    setLightSpaceMatrix(gLightSpaceMatrix);
    setInverseViewMatrix(gInverseViewMatrix);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gDepthMap);
    setShadowMap(1);
    glActiveTexture(GL_TEXTURE0);

    renderScene();
    
    disableSceneShader();

    glutSwapBuffers();
}'''
text = re.sub(display_search, display_replace, text)

with open('src/main.cpp', 'w') as f:
    f.write(text)
