QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets


TEMPLATE = app


CONFIG(debug, debug|release) {
    TARGET = TFG_debug
    DESTDIR = ../bin
    OBJECTS_DIR = ../build/debug
} else {
    TARGET = TFG
    DESTDIR = ../bin
    OBJECTS_DIR = ../build/release
}


INCLUDEPATH += $$(KINECTSDK10_DIR)/inc
LIBS += -L$$(KINECTSDK10_DIR)/lib/x86


INCLUDEPATH += $$(OPENCV_PATH)/include
LIBS += -L$$(OPENCV_PATH)/x86/vc10/lib


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
    WidgetRecorder.cpp

HEADERS += \
    globals.h \
    Utils.h \
    RenderUtils.h \
    DataStream.h \
    KinectManager.h \
    KinectStream.h \
    FixedFrameStream.h \
    RecordedStream.h \
    Calibration.h \
    Calibrator.h \
    Operation.h \
    Mode.h \
    ModeMeasure.h \
    MainWindow.h \
    SubWindow.h \
    WidgetOpenGL.h \
    WidgetColorView.h \
    WidgetDepthView.h \
    WidgetSceneView.h \
    WidgetRecorder.h \
    WidgetStreamManager.h

FORMS += WidgetRecorder.ui


MOC_DIR = $$OBJECTS_DIR
RCC_DIR = $$OBJECTS_DIR
UI_DIR = $$OBJECTS_DIR

win32:RC_FILE = resource.rc
win32:QMAKE_CXXFLAGS += -D_CRT_SECURE_NO_WARNINGS
