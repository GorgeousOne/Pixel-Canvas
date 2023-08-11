  #version 150
  uniform mat4 ortho;
  in vec2 Position;
  in vec2 UV;
  in vec4 Colour;
  out vec2 Frag_UV;
  out vec4 Frag_Colour;
  void main()
  {
    Frag_UV = UV;
    Frag_Colour = Colour;
    gl_Position = ortho*vec4(Position.xy,0,1);
  }
