import re
with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

# remove unused anons
text = re.sub(r'GLint gEffectModeLocation = -1;', 'GLint gEffectModeLocation = -1;\nGLint gEffectLightSpaceMatLoc2 = -1;\nGLint gEffectInvViewMatLoc2 = -1;\nGLint gEffectShadowMapLoc2 = -1;', text)

init_ext = '''        return false;
    }

    gEffectModeLocation = glGetUniformLocation(gShaderProgram, "uEffectMode");
    gEffectLightSpaceMatLoc2 = glGetUniformLocation(gShaderProgram, "uLightSpaceMatrix");
    gEffectInvViewMatLoc2 = glGetUniformLocation(gShaderProgram, "uInverseViewMatrix");
    gEffectShadowMapLoc2 = glGetUniformLocation(gShaderProgram, "uShadowMap");

    return true;'''
text = re.sub(r'        return false;\n    \}\n\n    gEffectModeLocation = glGetUniformLocation\(gShaderProgram, "uEffectMode"\);\n\n    return true;', init_ext, text)

setter_ext = '''void setSceneShaderEffect(int effectMode) {
    if (gShaderProgram && gEffectModeLocation != -1) {
        glUniform1i(gEffectModeLocation, effectMode);
    }
}

void setShadowMap(int textureUnit) {
    if (gShaderProgram && gEffectShadowMapLoc2 != -1) {
        glUniform1i(gEffectShadowMapLoc2, textureUnit);
    }
}

void setLightSpaceMatrix(const float* mat) {
    if (gShaderProgram && gEffectLightSpaceMatLoc2 != -1) {
        glUniformMatrix4fv(gEffectLightSpaceMatLoc2, 1, GL_FALSE, mat);
    }
}

void setInverseViewMatrix(const float* mat) {
    if (gShaderProgram && gEffectInvViewMatLoc2 != -1) {
        glUniformMatrix4fv(gEffectInvViewMatLoc2, 1, GL_FALSE, mat);
    }
}'''
text = re.sub(r'void setSceneShaderEffect\(int effectMode\) \{\n    if \(gShaderProgram && gEffectModeLocation != -1\) \{\n        glUniform1i\(gEffectModeLocation, effectMode\);\n    \}\n\}', setter_ext, text)

if 'void setShadowMap(int textureUnit)' not in text:
    text += '\n' + setter_ext

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)
