#version 130 compatibility

smooth out vec2 TexCoord;

void main()
{
  gl_Position = gl_Vertex;
  TexCoord = gl_MultiTexCoord0.st;
}
