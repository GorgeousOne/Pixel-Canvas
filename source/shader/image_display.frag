#version 330

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textureSampler;

void main() {
//    FragColor = texture(textureSampler, TexCoord);
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}