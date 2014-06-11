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

// Main window of the application.
// Contains a Multiple Document Interface (MDI) area that allows subwindows.
class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class StreamListModel;

    public:

        explicit MainWindow(QWidget *parent = 0);
        ~MainWindow();

        // Get the "singleton" kinect manager.
        KinectManager& getKinectManager()
        {
            return kinectManager;
        }

        // Get the current mode of interaction (may be null).
        const Ptr<Mode>* getModePointer() const
        {
            return &mode;
        }

        // Get the list of all streams (may contain nulls).
        const std::vector< Ptr<DataStream> >& getStreams() const
        {
            return streams;
        }

        // Gets the multi-device callibrations
        const SystemCalibration& getCalibration() const
        {
            return calibration;
        }


        // Adds a new stream, automatically opening the corresponding windows.
        int addStream(const Ptr<DataStream>& stream);

        // Closes a stream by a pointer to it.
        bool closeStream(DataStream* stream);

        // Closes a strea by its index.
        bool closeStream(int i);

        // Launches a new asynchronous Operation.
        void startOperation(Operation* op, std::function< void() > callback = nullptr);

    private:

        KinectManager kinectManager;

        std::vector< Ptr<DataStream> > streams;

        SystemCalibration calibration;

        // If configured with bullet
        #ifdef HAS_BULLET
            private:
                World world; // The physics world

            public:
                inline World& getWorld()
                {
                    return world;
                }

            private:
        #endif

        // The current mode of interaction (or null)
        Ptr<Mode> mode;

        // The current ongoing asynchronous operation.
        Ptr<AsyncOperation> currentOperation;


        // UI elements

        QStatusBar* statusBar;
        QLabel* statusBarText;
        QProgressBar* statusBarProgress;

        QToolBar* toolbar;

        QAction *actionDrawSkeleton, *actionSmoothSkeleton;

        QAction *actionPlayPause, *actionRestart, *actionAdvance;

        QIcon iconPlay, iconPause;

        QMdiArea* mdiArea; // The area containing the subwindows

        QMenu* menuModes;


        // Initializes the UI elements
        void setupUi();

        // Loads the configuration file (if exists) and opens the corresponding subwindows/streams.
        void initialize();

        // Add a new subwindow for the given widget and with the given title.
        void addSubWindow(SubWindowWidget* widget, const QString& title);

        // Opens the corresponding windows for the stream with index i.
        void openStreamWindows(int i);

        // Opens a kinect stream by its device index.
        void openKinect(int i);

        // Sets the current mode of interaction. "index" is the menu option index.
        void setMode(int index, Mode* mode);

        // Toggles between showing or not the skeletons in the given widget (if can display skeletons).
        void toggleSkeletonsOverlay(WidgetOpenGL* widget);

        // Called when a stream has been paused, unpaused, resetted, an option has changed, etc.
        void updateToolbar();


        // Returns the index of the given stream.
        int findStreamIndex(const Ptr<DataStream>& stream);

        //
        // UTILITY FUNCTIONS
        //
        
        // Returns the current selected subwindow stream, or null if no subwindow or has nostream
        inline Ptr<DataStream> getCurrentStream()
        {
            SubWindowWidget* w = dynamic_cast<SubWindowWidget*>(mdiArea->currentSubWindow()->widget());
            return (w != nullptr) ? w->getStream() : nullptr;
        }
        
        // Finds the (first) subwindow containing the templated type of widget.
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
        static T* defaultSubwindowFactory(MainWindow& window)
        {
            return new T(window);
        }

        // If a subwindow with the templated type of widget does not exists, opens it with the given title.
        // Otherwise it is closed and reopen.
        // (Note: the widget is constructed with a single argument, being a reference to the MainWindow)
        template<class T>
        inline void reopenSingletonSubwindow(const char* title, std::function<T* (MainWindow& window)> factory = &defaultSubwindowFactory<T>)
        {
            std::vector<QWidget*> toRemove;
            QList<QMdiSubWindow*> lst = mdiArea->subWindowList();
            for (auto it = lst.begin(); it != lst.end(); ++it) {
                T* ptr = dynamic_cast<T*>((*it)->widget());
                if (ptr != nullptr) toRemove.push_back(*it);
            }
            for (int i = 0; i < int(toRemove.size()); ++i) mdiArea->removeSubWindow(toRemove[i]);

            addSubWindow(factory(*this), title);
        }

    // QT slot functions, called as a result of interaction with the GUI elements (mainly the menus).
    public slots:

        void exit();

        void openKinect0() { openKinect(0); }
        void openKinect1() { openKinect(1); }

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

        void setSmoothSkeletons(bool);
        void setDrawSkeletons(bool);
        void setDrawTrajectory();
        
        void setModeNone();
        void setModeMeasure();

        void setStatusText(QString);

        void setOperationStatus(QString);
        void setOperationProgress(int, int);
        void operationFinished();

        void skeletonTraking();
        void skeletonWorking();

        void streamPlayPause();
        void streamRestart();
        void streamAdvance();
        

};


#endif
