import re

with open('include/render/shader.h', 'r') as f:
    header = f.read()

if 'setLightSpaceMatrix' not in header:
    header = header.replace('void shutdownSceneShader();', 'void shutdownSceneShader();\nvoid setLightSpaceMatrix(float* matrix);\nvoid setInverseViewMatrix(float* matrix);\nvoid setShadowMap(int texUnit);')
    with open('include/render/shader.h', 'w') as f:
        f.write(header)

