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

#ifdef HAS_BULLET
    #include "otger/World.h"
#endif

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

        #ifdef HAS_BULLET
            private:
                World world;

            public:
                inline World& getWorld()
                {
                    return world;
                }

            private:
        #endif

        Ptr<Mode> mode;

        Ptr<AsyncOperation> currentOperation;

        QStatusBar* statusBar;
        QLabel* statusBarText;
        QProgressBar* statusBarProgress;

        QAction* actionDrawSkeleton;

        QMdiArea* mdiArea;

        QMenu* menuModes;


        bool drawSkeletons;

        void setupUi();

        void initialize();

        void addSubWindow(SubWindowWidget* widget, const QString& title);

        void openStreamWindows(int i);

        void openKinect(int i);

        void setMode(int index, Mode* mode);

        void toggleSkeletonsOverlay(WidgetOpenGL* widget);

        int findStreamIndex(const Ptr<DataStream>& stream);


        template<class T>
        inline T* findSubwindowByType()
        {
            QList<QMdiSubWindow*> lst = mdiArea->subWindowList();
            for (auto it = lst.begin(); it != lst.end(); ++it) {
                T* ptr = dynamic_cast<T*>((*it)->widget());
                if (ptr != nullptr) return ptr;
            }
            return nullptr;
        }

        template<class T>
        inline void reopenSingletonSubwindow(const char* title)
        {
            std::vector<QWidget*> toRemove;
            QList<QMdiSubWindow*> lst = mdiArea->subWindowList();
            for (auto it = lst.begin(); it != lst.end(); ++it) {
                T* ptr = dynamic_cast<T*>((*it)->widget());
                if (ptr != nullptr) toRemove.push_back(*it);
            }
            for (int i = 0; i < int(toRemove.size()); ++i) mdiArea->removeSubWindow(toRemove[i]);

            addSubWindow(new T(*this), title);
        }

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
        void openAugmentedView();

        void openChessboardFinder();
        void openDepthCorrector();

        void changedSubwindow(QMdiSubWindow* win);

        void setDrawSkeletons(bool);

        void setModeNone();
        void setModeMeasure();

        void setStatusText(QString);
        void setStatusProgress(int, int);
        void operationFinished();

};


#endif
