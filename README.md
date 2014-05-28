
# Kinect TFG

### Dependencies

__Note:__ For now only Windows is supported due to the dependency on the official Kinect SDK.

  - Microsoft Visual Studio 2010 *(for the compiler, newer versions can be used as IDE)*
  - OpenCV 2.4.8 - [Download](http://opencv.org/downloads.html)
  - Kinect for Windows SDK v1.8 - [Download](http://go.microsoft.com/fwlink/?LinkID=323588)
  - Qt 5.2.1 for Windows 32-bit (VS 2010, OpenGL, 517 MB) *(other versions may work)* - [Download](http://qt-project.org/downloads)
  - (OPTIONAL) Bullet Physics Library 2.82 (r2704) - [Download Source](https://code.google.com/p/bullet/downloads/list) & [Download Binaries](https://dl.dropboxusercontent.com/u/26181106/Bullet_2.82_r2704_vs2010_Binaries.zip)
  - (OPTIONAL) LibXL for Windows 3.5.4 - [Download](http://www.libxl.com/download.html) - Only necessary to save skeleton data

### Configuration

Set the following environment variables, if not already present:

  - **QTDIR** to QT installation (e.g. *C:/Qt/5.2.1/msvc2010_opengl*)
  - **OPENCV_PATH** to OpenCV build (e.g. *C:/opencv/build*)
  - **KINECTSDK10_DIR** to Kinect SDK installation (e.g. *C:/Program Files/Microsoft SDKs/Kinect/v1.8/*)
  - (OPTIONAL) **BULLET_DIT** to Bullet Physics Library folder (e.g *C:/bullet-2.82-r2704*)
  - (OPTIONAL) **LIBXL_DIR** to libxl folder (e.g *C:/libxl*)
  
Additionally, change the **PATH** variable in order to include the folders containing the required *.dll*
(or copy instead these files to the _/bin_ folder of the project).
The OpenCV ones should be under its build directory (e.g. *C:/opencv/build/x86/vc10/bin*).
The QT and Kinect ones are probably already present inside a folder in PATH.

For the optional libraries (modules), it is necessary to create a file named "config.pro" at the project root.
This file will be ignored by **git**, and inside you can include further personal options for the project.
The options for enabling optional libraries are the following:

  - "WITH_BULLET = 1": Enable Bullet (physics library) support, for physics simulation in the augmented reality scene.
  - "WITH_LIBXL = 1": For exporting Excel data about the skeletons.


### Visual Studio project

  Executing the provided __GenerateVCXPROJ.bat__ will generate under the __/src__ folder a Visual Studio 2010 project (.vcxproj),
  configured to compile executing the required additional QT steps. Doing so allows to use the built-in Visual Studio
  debugger in the development. Newer versions of the Visual Studio may be used as editors, but they should employ the Visual Studio 2010 compiler toolchain.
  
  __Note:__ If new files are to be added, they should be included in the _/src/TFG.pro_ file manually with a text editor or using QT Creator, and then regenerate the Visual Studio project. Generally any manual change to the Visual Studio project configuration should be avoided, in favour of modifying the .pro file.
