import re

with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

# Replace variables
text = text.replace('GLint gEffectModeLocation = -1;', 'GLint gEffectModeLocation = -1;\nGLint gEffectLightSpaceMatLoc = -1;\nGLint gEffectInvViewMatLoc = -1;\nGLint gEffectShadowMapLoc = -1;')

v_shader_search = r'''void main\(\).*?gl_Position = ftransform\(\);\n}'''
v_shader_replace = r'''varying vec4 vFragPosLightSpace;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uInverseViewMatrix;

void main() {
    vec4 eyePos = gl_ModelViewMatrix * gl_Vertex;
    vEyePos = eyePos.xyz;
    vNormal = normalize(gl_NormalMatrix * gl_Normal);
    vColor = gl_Color;
    vObjectPos = gl_Vertex.xyz;
    vObjectNormal = normalize(gl_Normal);
    
    vec4 worldPos = uInverseViewMatrix * eyePos;
    vFragPosLightSpace = uLightSpaceMatrix * worldPos;

    gl_Position = ftransform();
}'''

text = re.sub(v_shader_search, v_shader_replace, text, flags=re.DOTALL)

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)
