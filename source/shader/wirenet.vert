#version 330

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoord;

uniform mat4 modelMatrix;

void main() {
    gl_Position = modelMatrix * vec4(inPosition, 0.0, 1.0);
}