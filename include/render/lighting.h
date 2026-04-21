#ifndef LIGHTING_H
#define LIGHTING_H

void initLighting();
void positionLights();
void setCeilingLightsEnabled(bool enabled);
bool areCeilingLightsEnabled();
void getShadowLightDirection(float outDirection[3]);
void getShadowLightAmbient(float outColor[4]);
void getShadowLightDiffuse(float outColor[4]);
void getShadowLightSpecular(float outColor[4]);
void getScreenLightPosition(float outPosition[3]);
void getScreenLightDirection(float outDirection[3]);
float getScreenLightSpotCutoffDegrees();
void setMaterial(float r, float g, float b, float shininess, float specularStrength, float emissionStrength = 0.0f);

#endif
