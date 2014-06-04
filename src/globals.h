//
// Includes common headers and declares global functions / defines.
//
#ifndef GLOBALS_H
#define GLOBALS_H

#pragma once

#ifdef _MSC_VER

    // Disable "Unreferenced formal parameter" warning
    #pragma warning(disable : 4100)

#endif


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

#include <Windows.h>
#include <Shlobj.h>

#undef min
#undef max


//
// Kinect
//

#include <NuiApi.h>

#ifdef HAS_KINECT_TOOLKIT
    #include <KinectInteraction.h>
#endif


//
// OpenCV
//

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>



//
// QT
//

#include <QDebug>


//
// OpenGL
//

#include <GL/gl.h>
#include <GL/glu.h>

// gl.h/QtOpenGL conflict fix
#define GLdouble GLdouble

#include <QtOpenGL>



//
// Misc
//

// Shared Pointer (pointer with reference count and auto-delete)
using cv::Ptr;

// Unique Pointer (pointer with auto-delete)
using std::unique_ptr;



// Defines for better readability

// OUT: indicates that a function argument/parameter is to be changed to output a result.
#ifndef OUT
    #define OUT
#endif

// INOUT: indicates that a function argument/parameter may be both read and modified
#ifndef INOUT
    #define INOUT
#endif



// Common operations

#ifndef MAX
    #define MAX(x, y) (((x) > (y))? (x) : (y))
#endif

#ifndef MIN
    #define MIN(x, y) (((x) < (y))? (x) : (y))
#endif


// Common constants

const float PI = 3.14159265358979323846f;

const float DEG2RAD = PI / 180.0f;
const float RAD2DEG = 180.0f / PI;


#endif


