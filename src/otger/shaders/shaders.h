
// This file is generated automatically.

const char SHADER_2D_FRAGMENT[] = ""
"#version 130 compatibility\n"
"\n"
"const float zNear = -0.01;\n"
"const float zFar = -20.0;\n"
"\n"
"uniform sampler2D Color;\n"
"uniform usampler2D Depth;\n"
"\n"
"smooth in vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = texture(Color, TexCoord);\n"
"  uint z_mm = texture(Depth, TexCoord).r;\n"
"  float z_m = float(z_mm) / 1000.0;\n"
"  float z = ((zFar + zNear) + (2.0*zFar*zNear)/z_m) / (zFar - zNear);\n"
"	gl_FragDepth = (z + 1.0) / 2.0;\n"
"}\n"
;

const char SHADER_DEFAULT_FRAGMENT[] = ""
"#version 120\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = gl_Color;\n"
"}\n"
;

const char SHADER_2D_VERTEX[] = ""
"#version 130 compatibility\n"
"\n"
"smooth out vec2 TexCoord;\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position = gl_Vertex;\n"
"	TexCoord = gl_MultiTexCoord0.st;\n"
"}\n"
;

const char SHADER_DEFAULT_VERTEX[] = ""
"#version 120\n"
"\n"
"void main()\n"
"{\n"
"	gl_Position    = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
"	gl_FrontColor  = gl_Color;\n"
"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
"}\n"
;

