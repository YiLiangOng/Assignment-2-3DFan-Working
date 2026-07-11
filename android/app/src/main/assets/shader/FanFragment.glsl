#version 300 es
precision mediump float;

in float vShade;

// Uniform matching Fan::InitModel()
uniform vec3 PARTCOLOR;

// Final output color for the pixel
out vec4 FragColor;

void main() {
    // Multiply the base part color by the per-vertex grayscale shade
    FragColor = vec4(PARTCOLOR * vShade, 1.0);
}