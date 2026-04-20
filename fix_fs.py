with open("src/render/shader.cpp", "r") as f:
    text = f.read()

# Add missing uniform to fragment shader
f_shader_idx = text.find('uniform sampler2D uShadowMap;')
if f_shader_idx != -1 and 'uniform mat4 uInverseViewMatrix;'[6:] not in text[text.find('kFragmentShaderSource'):]:
    text = text.replace('uniform sampler2D uShadowMap;', 'uniform sampler2D uShadowMap;\nuniform mat4 uInverseViewMatrix;')

with open("src/render/shader.cpp", "w") as f:
    f.write(text)

