#version 300 es

// Input attributes matching Fan::createCube() locations
layout(location = 0) in vec3 aPosition;
layout(location = 1) in float aShade;

// Uniform matching Fan::InitModel()
uniform mat4 MODELVIEWPROJECTIONMATRIX;

out float vShade;

void main() {
    gl_Position = MODELVIEWPROJECTIONMATRIX * vec4(aPosition, 1.0);
    vShade = aShade;
}