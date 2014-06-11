#version 130 compatibility

uniform sampler2D colorMap;
uniform usampler2D depthMap;

//uniform float zNear;
//uniform float zFar;

const float zNear = -0.01;
const float zFar = -20.0;

smooth in vec2 texCoord;

void main()
{
  gl_FragColor = texture(colorMap, texCoord);
  uint z_mm = texture(depthMap, texCoord).r;
  float z_m = float(z_mm) / 1000.0;
  //float z = ((2.0*zFar*zNear)/z_m - (zFar + zNear)) / (zNear - zFar);
  float z = ((zFar + zNear) + (2.0*zFar*zNear)/z_m) / (zFar - zNear);
  gl_FragDepth = (z + 1.0) / 2.0;
}
