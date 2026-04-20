with open('src/render/shader.cpp', 'r') as f:
    text = f.read()

text = text.replace('float intensity = 5.0; // Global brightness scalar for the grid', 'float intensity = 20.0; // Global brightness scalar for the grid')

with open('src/render/shader.cpp', 'w') as f:
    f.write(text)
