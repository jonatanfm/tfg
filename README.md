
# TFG

### Dependencies

__Note:__ For now only Windows is supported.

  - Microsoft Visual Studio 2010 *(For the compiler)*
  - OpenCV 2.4.8 - [Download](http://opencv.org/downloads.html)
  - Kinect for Windows SDK v1.8 - [Download](http://go.microsoft.com/fwlink/?LinkID=323588)
  - Qt 5.2.1 for Windows 32-bit (VS 2010, OpenGL, 517 MB) *(other versions may work)* - [Download](http://qt-project.org/downloads)

### Configuration

Set the following environment variables, if not already present:

  - **QTDIR** to QT installation (e.g. *C:/Qt/5.2.1/msvc2010_opengl*)
  - **OPENCV_PATH** to OpenCV build (e.g. *C:/opencv/build*)
  - **KINECTSDK10_DIR** to Kinect SDK installation (e.g. *C:/Program Files/Microsoft SDKs/Kinect/v1.8/*)

Additionally, change the **PATH** variable in order to include the folders containing the required *.dll*
(or copy instead these files to the _/bin_ folder of the project).
The OpenCV ones should be under its build directory (e.g. *C:/opencv/build/x86/vc10/bin*).
The QT and Kinect ones are probably already present inside a folder in PATH.


### Visual Studio project

  Executing the attached __GenerateVCXPROJ.bat__ will generate under the __/src__ folder a Visual Studio 2010 project (.vcxproj),
  configured to compile executing the required additional QT steps. Doing so allows to use the built-in Visual Studio
  debugger in the development. Newer versions of the Visual Studio may be used, but they should use the Visual Studio 2010 compiler.
  
  __Note:__ If new files are to be added, they should be included in the _/src/TFG.pro_ file manually or using QT Creator, and then regenerate the Visual Studio project. Generally any manual change to the Visual Studio project configuration should be avoided, in favour of modifying the .pro file.
