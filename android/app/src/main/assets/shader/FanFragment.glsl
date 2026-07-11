#version 300 es
precision mediump float;

in float Shade;

// Uploaded once per part via glUniform3fv before each draw call -- this is
// what gives the base/pole/hub/blades their seven distinct colours even
// though they all share the same cube geometry.
uniform vec3 PARTCOLOR;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(PARTCOLOR * Shade, 1.0);
}
