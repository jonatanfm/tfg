#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QMdiSubWindow>

#include "globals.h"
#include "RenderUtils.h"

#include "KinectManager.h"
#include "KinectStream.h"

#include "FixedFrameStream.h"

#include "SubWindow.h"
#include "WidgetOpenGL.h"

#include "Mode.h"

class App;
class Calibrator;

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class StreamListModel;

    public:
        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        KinectManager& getKinectManager()
        {
            return kinectManager;
        }

        Ptr<Mode> getMode() const
        {
            return mode;
        }

        const std::vector< Ptr<DataStream> >& getStreams()
        {
            return streams;
        }

        bool getDrawSkeletons() const
        {
            return drawSkeletons;
        }

        void closeStream(int i);

    private:

        KinectManager kinectManager;

        std::vector< Ptr<DataStream> > streams;

        Ptr<Mode> mode;

        Ptr<Calibrator> calibrator;

        QStatusBar* statusbar;
        QMdiArea* mdiArea;

        QMenu* menuModes;

        bool drawSkeletons;

        void setupUi();

        void initialize();

        void addSubwindow(QWidget* widget, const QString& title);

        void openStreamWindows(int i);

        void openKinect(int i);

        void setMode(int index, Mode* mode);

    public slots:

        void exit();

        void openKinect1() { openKinect(0); }
        void openKinect2() { openKinect(1); }

        void openRecorder();

        void openStreamManager();

        void openRecordedStream();

        void openImageStream();

        void changedSubwindow(QMdiSubWindow* win);

        void setDrawSkeletons(bool);

        void setModeNone();
        void setModeMeasure();

        void calibrate();


};

#endif
