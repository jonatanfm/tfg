#ifndef GLOBALS_H
#define GLOBALS_H

#pragma once

// Disable "Unreferenced formal parameter"
#pragma warning(disable : 4100)


//
// C/C++
//

#include <cstdio>
#include <cstdint>
#include <cmath>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <functional>
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
    #pragma comment(lib, "opencv_core249d")
    #pragma comment(lib, "opencv_imgproc249d")
    #pragma comment(lib, "opencv_calib3d249d")
    #pragma comment(lib, "opencv_highgui249d")


    // If static libs
    
    /*#pragma comment(lib, "opencv_contrib249d")
    #pragma comment(lib, "opencv_legacy249d")
    #pragma comment(lib, "opencv_ml249d")
    #pragma comment(lib, "opencv_objdetect249d")
    #pragma comment(lib, "opencv_calib3d249d")
    #pragma comment(lib, "opencv_video249d")
    #pragma comment(lib, "opencv_features2d249d")
    #pragma comment(lib, "opencv_highgui249d")
    #pragma comment(lib, "opencv_flann249d")
    #pragma comment(lib, "opencv_imgproc249d")
    #pragma comment(lib, "opencv_core249d")

    #pragma comment(lib, "libjpegd")
    #pragma comment(lib, "libpngd")
    #pragma comment(lib, "libtiffd")
    #pragma comment(lib, "libjasperd")
    #pragma comment(lib, "IlmImfd")

    #pragma comment(lib, "zlibd")*/


#else
    #pragma comment(lib, "opencv_core249")
    #pragma comment(lib, "opencv_imgproc249")
    #pragma comment(lib, "opencv_calib3d249")
    #pragma comment(lib, "opencv_highgui249")
#endif


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

using cv::Ptr;

using std::unique_ptr;

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


const float PI = 3.14159265358979323846f;

const float DEG2RAD = PI / 180.0f;
const float RAD2DEG = 180.0f / PI;


#endif


