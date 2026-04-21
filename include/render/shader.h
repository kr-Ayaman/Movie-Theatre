#ifndef SHADER_H
#define SHADER_H

constexpr int kSceneShaderEffectDefault = 0;
constexpr int kSceneShaderEffectCurtain = 1;
constexpr int kSceneShaderEffectBrick = 2;
constexpr int kSceneShaderEffectCushion = 3;
constexpr int kSceneShaderEffectCeiling = 4;
constexpr int kSceneShaderEffectGate = 5;
constexpr int kSceneShaderEffectMetal = 6;
constexpr int kShadowLightModeNone = 0;
constexpr int kShadowLightModeCeiling = 1;
constexpr int kShadowLightModeScreen = 2;

bool initSceneShader();
void enableSceneShader();
void disableSceneShader();
void setSceneShaderEffect(int effectMode);
bool isSceneShaderActive();
void shutdownSceneShader();
void setLightSpaceMatrix(float* matrix);
void setInverseViewMatrix(float* matrix);
void setShadowMap(int texUnit);
void setShadowLightDirection(const float direction[3]);
void setShadowLightPosition(const float position[3]);
void setShadowLightAmbient(const float color[4]);
void setShadowLightDiffuse(const float color[4]);
void setShadowLightSpecular(const float color[4]);
void setShadowLightMode(int mode);
void setShadowLightSpotCutoff(float cosCutoff);
void setShadowLightEnabled(bool enabled);
void setParallelProjectionEnabled(bool enabled);

#endif
