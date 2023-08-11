#version 150 
uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Colour;
out vec4 FragColor;
void main()
{
  FragColor = Frag_Colour * texture( Texture, Frag_UV.st);
}
