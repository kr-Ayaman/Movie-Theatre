#ifndef SHADER_H
#define SHADER_H

constexpr int kSceneShaderEffectDefault = 0;
constexpr int kSceneShaderEffectCurtain = 1;
constexpr int kSceneShaderEffectBrick = 2;
constexpr int kSceneShaderEffectCushion = 3;
constexpr int kSceneShaderEffectCeiling = 4;

bool initSceneShader();
void enableSceneShader();
void disableSceneShader();
void setSceneShaderEffect(int effectMode);
bool isSceneShaderActive();
void shutdownSceneShader();

#endif