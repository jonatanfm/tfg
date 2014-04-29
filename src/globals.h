#ifndef GLOBALS_H
#define GLOBALS_H

#pragma once

// Disable "Unreferenced formal parameter"
#pragma warning(disable : 4100)


#define COLOR_FRAME_WIDTH  640
#define COLOR_FRAME_HEIGHT 480

#define DEPTH_FRAME_WIDTH  640
#define DEPTH_FRAME_HEIGHT 480


//
// C/C++
//

#include <cstdio>
#include <cstdint>

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>


//
// Windows
//

#ifndef WIN32_LEAN_AND_MEAN
  //#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#undef min
#undef max


//
// Kinect
//

#include <NuiApi.h>


//
// OpenCV
//

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifdef _DEBUG
    #pragma comment(lib, "opencv_core248d")
    #pragma comment(lib, "opencv_imgproc248d")
    #pragma comment(lib, "opencv_calib3d248d")
    #pragma comment(lib, "opencv_highgui248d")
#else
    #pragma comment(lib, "opencv_core248")
    #pragma comment(lib, "opencv_imgproc248")
    #pragma comment(lib, "opencv_calib3d248")
    #pragma comment(lib, "opencv_highgui248")
#endif

using cv::Ptr;


//
// QT
//

#include <QDebug>


//
// OpenGL
//

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")
#include <gl/GL.h>
#include <gl/GLU.h>

// GL.h/QtOpenGL conflict fix
#define GLdouble GLdouble

#include <QtOpenGL>





//
// Misc
//

#ifndef OUT
    #define OUT
#endif

#ifndef INOUT
    #define INOUT
#endif

#ifndef MAX
    #define MAX(x, y) (((x) > (y))? (x) : (y))
#endif

#ifndef MIN
    #define MIN(x, y) (((x) < (y))? (x) : (y))
#endif


#endif


