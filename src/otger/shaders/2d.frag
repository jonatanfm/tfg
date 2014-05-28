#version 130 compatibility

const float zNear = -0.01;
const float zFar = -20.0;

uniform sampler2D Color;
uniform usampler2D Depth;

smooth in vec2 TexCoord;

void main()
{
	gl_FragColor = texture(Color, TexCoord);
  uint z_mm = texture(Depth, TexCoord).r;
  float z_m = float(z_mm) / 1000.0;
  float z = ((zFar + zNear) + (2.0*zFar*zNear)/z_m) / (zFar - zNear);
	gl_FragDepth = (z + 1.0) / 2.0;
}
