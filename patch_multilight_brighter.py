import re

with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

# Make the array of lights much brighter since they get attenuated
text = text.replace('float atten = 1.0 / (1.0 + 0.03 * dist + 0.005 * dist * dist);', 'float atten = 1.0 / (1.0 + 0.01 * dist + 0.001 * dist * dist);')
text = text.replace('vec3 l_amb = gl_LightSource[0].ambient.rgb * gl_FrontMaterial.ambient.rgb / 77.0;', 'vec3 l_amb = gl_LightSource[0].ambient.rgb * gl_FrontMaterial.ambient.rgb / 20.0;')
text = text.replace('vec3 l_diff = gl_LightSource[0].diffuse.rgb * gl_FrontMaterial.diffuse.rgb / 77.0;', 'vec3 l_diff = gl_LightSource[0].diffuse.rgb * gl_FrontMaterial.diffuse.rgb / 15.0;')
text = text.replace('vec3 l_spec = gl_LightSource[0].specular.rgb * gl_FrontMaterial.specular.rgb / 77.0;', 'vec3 l_spec = gl_LightSource[0].specular.rgb * gl_FrontMaterial.specular.rgb / 15.0;')

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)

