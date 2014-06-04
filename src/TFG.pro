QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

message(".: Generating :.")

TEMPLATE = app

# Set debug/Release folders
CONFIG(debug, debug|release) {
    TARGET = TFG_debug
    DESTDIR = ../bin
    OBJECTS_DIR = ../build/debug
} else {
    TARGET = TFG
    DESTDIR = ../bin
    OBJECTS_DIR = ../build/release
}

#INCLUDEPATH += ../include


# Kinect SDK
INCLUDEPATH += $$(KINECTSDK10_DIR)/inc
LIBS += -L$$(KINECTSDK10_DIR)/lib/x86 -lkinect10

# Kinect Developer Toolkit
KINECT_TOOLKIT_DIR = "$$(KINECTSDK10_DIR)/../Developer Toolkit v1.8.0"
exists($$KINECT_TOOLKIT_DIR) {
    message("Kinect Toolkit found")
    INCLUDEPATH += "$$(KINECT_TOOLKIT_DIR)/inc"
    LIBS += -L"$$(KINECT_TOOLKIT_DIR)/lib/x86" -lKinectInteraction180_32
    DEFINES += HAS_KINECT_TOOLKIT
}


# OpenGL
LIBS += -lopengl32 -lglu32


# OpenCV
INCLUDEPATH += $$(OPENCV_PATH)/include
LIBS += -L$$(OPENCV_PATH)/x86/vc10/lib
CONFIG(debug, debug|release) {
    LIBS += -lopencv_core249d -lopencv_imgproc249d -lopencv_calib3d249d -lopencv_highgui249d
} else {
    LIBS += -lopencv_core249 -lopencv_imgproc249 -lopencv_calib3d249 -lopencv_highgui249
}


SOURCES += \
    main.cpp \
    RenderUtils.cpp \
    KinectManager.cpp \
    KinectStream.cpp \
    FixedFrameStream.cpp \
    RecordedStream.cpp \
    Calibrator.cpp \
    MainWindow.cpp \
    WidgetOpenGL.cpp \
    WidgetRecorder.cpp \
    otger/DepthCorrector.cpp \
    jonatan/SkeletonStudy.cpp

HEADERS += \
    globals.h \
    Utils.h \
    RenderUtils.h \
    Data.h \
    DataStream.h \
    AsyncStream.h \
    KinectManager.h \
    KinectStream.h \
    FixedFrameStream.h \
    RecordedStream.h \
    ChessboardDetectorStream.h \
    Calibration.h \
    Calibrator.h \
    Operation.h \
    SkeletonIO.h \
    Mode.h \
    ModeMeasure.h \
    MainWindow.h \
    SubWindow.h \
    WidgetOpenGL.h \
    WidgetColorView.h \
    WidgetDepthView.h \
    WidgetSceneView.h \
    WidgetRecorder.h \
    WidgetStreamManager.h \
    otger/DepthCorrector.h \
    jonatan/SkeletonStudy.h

FORMS += WidgetRecorder.ui

exists(../config.pro) {
    include(../config.pro)
}

# Bullet Physics Library
!isEmpty(WITH_BULLET) {
    message("Configured with Bullet")

    SOURCES += \
        otger/World.cpp \
        otger/WidgetAugmentedView.cpp
        
    HEADERS += \
        otger/World.h \
        otger/WidgetAugmentedView.h
    
    INCLUDEPATH += $$(BULLET_DIR)/src
    LIBS += -L$$(BULLET_DIR)/lib
        
    CONFIG(debug, debug|release) {
        LIBS += -llinearmath_vs2010_debug -lbulletcollision_vs2010_debug -lbulletdynamics_vs2010_debug
    } else {
        LIBS += -llinearmath_vs2010 -lbulletcollision_vs2010 -lbulletdynamics_vs2010
    }
    
    DEFINES += HAS_BULLET
}

# LibXL (Excel .xls exporting)
!isEmpty(WITH_LIBXL) {
    message("Configured with LIBXL")

    INCLUDEPATH += $$(LIBXL_DIR)/include_cpp
    LIBS += -L$$(LIBXL_DIR)/lib libxl.lib
    
    DEFINES += HAS_LIBXL
}


# Set build directories
MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR

RESOURCES = ../res/resources.qrc

# Windows specific
win32:RC_FILE = ../res/resource.rc
win32:QMAKE_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS
