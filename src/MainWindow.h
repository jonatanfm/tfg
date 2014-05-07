#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QMdiSubWindow>

#include "globals.h"
#include "RenderUtils.h"

#include "KinectManager.h"
#include "KinectStream.h"

#include "SubWindow.h"
#include "WidgetOpenGL.h"

#include "Calibration.h"

#include "Mode.h"

class App;
class Calibrator;

class AsyncOperation;
class Operation;

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

        const std::vector< Ptr<DataStream> >& getStreams() const
        {
            return streams;
        }

        const SystemCalibration& getCalibration() const
        {
            return calibration;
        }

        bool getDrawSkeletons() const
        {
            return drawSkeletons;
        }

        int addStream(const Ptr<DataStream>& stream);

        bool closeStream(DataStream* stream);

        bool closeStream(int i);

        void startOperation(Operation* op, std::function< void() > callback = nullptr);

    private:

        KinectManager kinectManager;

        std::vector< Ptr<DataStream> > streams;

        SystemCalibration calibration;

        Ptr<Mode> mode;

        Ptr<AsyncOperation> currentOperation;

        QStatusBar* statusBar;
        QLabel* statusBarText;
        QProgressBar* statusBarProgress;

        QMdiArea* mdiArea;

        QMenu* menuModes;


        bool drawSkeletons;

        void setupUi();

        void initialize();

        void addSubWindow(SubWindowWidget* widget, const QString& title);

        void openStreamWindows(int i);

        void openKinect(int i);

        void setMode(int index, Mode* mode);

        int findStreamIndex(const Ptr<DataStream>& stream);


    public slots:

        void exit();

        void openKinect1() { openKinect(0); }
        void openKinect2() { openKinect(1); }

        void openRecorder();

        void openStreamManager();

        void updateStreamManager();

        void openRecordedStream();

        void openImageStream();

        void openSceneView();

        void openChessboardFinder();

        void changedSubwindow(QMdiSubWindow* win);

        void setDrawSkeletons(bool);

        void setModeNone();
        void setModeMeasure();

        void setStatusText(QString);
        void setStatusProgress(int, int);
        void operationFinished();

};


#endif
