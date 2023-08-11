#version 330

layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoord;

out vec2 TexCoord;

uniform mat4 modelMatrix;

void main() {
    TexCoord = inTexCoord;
    gl_Position = modelMatrix * vec4(inPosition, 0.0, 1.0);
}