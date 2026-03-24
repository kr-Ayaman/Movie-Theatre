#ifndef LIGHTING_H
#define LIGHTING_H

void initLighting();
void positionLights();
void setMaterial(float r, float g, float b, float shininess, float specularStrength, float emissionStrength = 0.0f);

#endif
