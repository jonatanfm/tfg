#version 130 compatibility

uniform sampler2D Color;
uniform usampler2D Depth;

uniform float zNear;
uniform float zFar;

smooth in vec2 TexCoord;

void main()
{
  gl_FragColor = texture(Color, TexCoord);
  uint z_mm = texture(Depth, TexCoord).r;
  float z_m = float(z_mm) / 1000.0;
  float z = ((2.0*zFar*zNear)/z_m - (zFar + zNear)) / (zNear - zFar);
  gl_FragDepth = (z + 1.0) / 2.0;
}
