#version 300 es

// Fan.cpp uploads the shared cube once with these two attributes at fixed
// locations (Chapter 3 tutorial 1 style) -- no glGetAttribLocation needed.
layout(location = 0) in vec4 VertexPosition;
layout(location = 1) in float VertexShade;

// One MVP per part, uploaded fresh before each of the seven draw calls.
uniform mat4 MODELVIEWPROJECTIONMATRIX;

out float Shade;

void main()
{
    gl_Position = MODELVIEWPROJECTIONMATRIX * VertexPosition;
    Shade = VertexShade;
}
