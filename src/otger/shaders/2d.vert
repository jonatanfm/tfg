#version 130 compatibility

smooth out vec2 texCoord;

void main()
{
  gl_Position = gl_Vertex;
  texCoord = gl_MultiTexCoord0.st;
}
