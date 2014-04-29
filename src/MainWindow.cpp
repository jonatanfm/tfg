#include "MainWindow.h"

#include "WidgetColorView.h"
#include "WidgetDepthView.h"
#include "WidgetRecorder.h"
#include "WidgetStreamManager.h"

#include "Calibrator.h"

#include "ModeMeasure.h"


template<class T>
inline T* findSubwindowByType(QMdiArea* area)
{
    QList<QMdiSubWindow*> lst = area->subWindowList();
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        T* ptr = dynamic_cast<T*>((*it)->widget());
        if (ptr != nullptr) return ptr;
    }
    return nullptr;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    drawSkeletons(true)
{
    setupUi();

    setModeNone();

    initialize();
}

MainWindow::~MainWindow()
{
    
}

void MainWindow::initialize()
{
    for (int i = 0; i < kinectManager.getSensorCount(); ++i) {
        openKinect(i);
    }

    std::string lineStr, str;
    std::ifstream file("config.txt");
    while (file && std::getline(file, lineStr)) {
        std::istringstream line(lineStr);
        if (line >> str) {
            if (str == "stream") {
                if (line >> str) {
                    if (str == "fixed") {
                        std::string sColor, sDepth, sSkeleton;
                        line >> sColor >> sDepth >> sSkeleton;
                        Ptr<DataStream> stream(new FixedFrameStream(sColor, sDepth, sSkeleton));
                        streams.push_back(stream);
                        openStreamWindows(streams.size() - 1);
                    }
                }
            }
        }
    }
}

void MainWindow::exit()
{
    close();
    QApplication::quit();
}


void MainWindow::setMode(int index, Mode* newMode)
{
    mode = newMode;

    QList<QAction*> actions = menuModes->actions();
    for (int i = 0; i < actions.length(); ++i) {
        actions[i]->setCheckable(true);
        actions[i]->setChecked(index == i);
    }
}

void MainWindow::setModeNone()
{
    setMode(0, nullptr);
}

void MainWindow::setModeMeasure()
{
    setMode(1, new MeasureMode(this));
}


void MainWindow::addSubwindow(QWidget* widget, const QString& title)
{
    SubWindow* win = new SubWindow(this);
    win->setWidget(widget);
    win->setWindowTitle(title);
    win->setAttribute(Qt::WA_DeleteOnClose);
    //win->setWindowIcon(this->windowIcon());
    mdiArea->addSubWindow(win);
    win->show();
}

void MainWindow::setDrawSkeletons(bool draw)
{
    drawSkeletons = draw;

    QMdiSubWindow* win = mdiArea->currentSubWindow();
    if (win == nullptr) return;

    WidgetOpenGL* widget = dynamic_cast<WidgetOpenGL*>(win->widget());
    if (widget != nullptr) {
        if (widget->hasOverlay("skeleton")) {
            widget->removeOverlay("skeleton");
        }
        else {
            switch (widget->getType()) {
                case WidgetOpenGL::ColorFrameView:
                    widget->addOverlay("skeleton", [] (WidgetOpenGL* w) -> bool {
                        NUI_SKELETON_FRAME skeleton;
                        if (((WidgetColorView*)w)->getStream()->getSkeletonFrame(skeleton)) {
                            RenderUtils::drawSkeletons(skeleton, true);
                        }
                        return true;
                    });
                    break;

                case WidgetOpenGL::DepthFrameView:
                    widget->addOverlay("skeleton", [] (WidgetOpenGL* w) -> bool {
                        NUI_SKELETON_FRAME skeleton;
                        if (((WidgetDepthView*)w)->getStream()->getSkeletonFrame(skeleton)) {
                            RenderUtils::drawSkeletons(skeleton, false);
                        }
                        return true;
                    });
                    break;
            }
        }
    }
}

void MainWindow::changedSubwindow(QMdiSubWindow* /*win*/)
{

}


void MainWindow::closeStream(int i)
{
    if (i < int(streams.size())) {
        streams[i] = nullptr;
    }
}

void MainWindow::openStreamWindows(int i)
{
    if (i >= int(streams.size())) return;

    Ptr<DataStream> stream = streams[i];

    QString name = QString::fromStdString(stream->getName());

    if (stream->hasColor())
    {
        WidgetColorView* colorView = new WidgetColorView(this, stream);
        addSubwindow(colorView, "#" + QString::number(i) + " - " + name + " - Color");
    }

    if (stream->hasDepth())
    {
        WidgetDepthView* depthView = new WidgetDepthView(this, stream);
        addSubwindow(depthView, "#" + QString::number(i) + " - " + name + " - Depth");
    }
}

void MainWindow::openKinect(int i)
{
    if (kinectManager.getSensorCount() <= i) return;

    Ptr<KinectStream> stream = kinectManager.getStream(i);
    if (stream == nullptr) return;

    for (int j = 0; j < int(streams.size()); ++j) {
        if (streams[j] == stream) return;
    }

    streams.push_back(stream);
    openStreamWindows(streams.size() - 1);
}

void MainWindow::openRecorder()
{
    addSubwindow(new WidgetRecorder(*this), "Recorder");
}

void MainWindow::openStreamManager()
{
    WidgetStreamManager* w = findSubwindowByType<WidgetStreamManager>(mdiArea);
    if (w != nullptr) {
        w->refresh();
        ((QWidget*) w->parent())->setFocus();
    }
    else {
        w = new WidgetStreamManager(*this);
        w->refresh();
        addSubwindow(w, "Stream Manager");
    }
}

void MainWindow::openImageStream()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList filters;
    filters << "Image files (*.png *.bmp *.xpm *.jpg)" << "Any files (*)";
    dialog.setNameFilters(filters);
    if (dialog.exec()) {
        QStringList list = dialog.selectedFiles();

        QString color = "", depth = "", skeleton = "";

        for (int i = 0; i < list.size(); ++i) {
            if (list[i].indexOf("skeleton") != -1) skeleton = list[i];
            else if (list[i].indexOf("depth") != -1) depth = list[i];
            else color = list[i];
        }

        Ptr<DataStream> stream(new FixedFrameStream(color.toStdString(), depth.toStdString(), skeleton.toStdString()));
        streams.push_back(stream);

        openStreamWindows(streams.size() - 1);
    }
}

void MainWindow::openRecordedStream()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Stream Videos (*.avi *.wmv *.mp4 *.bin)");
    if (dialog.exec()) {
        QStringList list = dialog.selectedFiles();
    }
}


void MainWindow::calibrate()
{
    if (calibrator != nullptr) return;

    //calibrator = new Calibrator();
    //calibrator->addStream(streams[0]);
    ////calibrator->addStream(streams[1]);
    //calibrator->calibrateStream();
}



#pragma region Setup UI

#define MENU(_text) \
    menu = new QMenu(QApplication::translate("MainWindow", (_text), 0), menubar); \
    menubar->addAction(menu->menuAction());

#define ACTION(_text, _slot) { \
        action = new QAction(QApplication::translate("MainWindow", (_text), 0), this); \
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(_slot)); \
        menu->addAction(action); \
    }

#define ACTION_2(_text, _signal, _slot) { \
        action = new QAction(QApplication::translate("MainWindow", (_text), 0), this); \
        QObject::connect(action, SIGNAL(_signal), this, SLOT(_slot)); \
        menu->addAction(action); \
    }

void MainWindow::setupUi()
{
    if (objectName().isEmpty()) setObjectName(QStringLiteral("MainWindow"));

    setWindowTitle(QApplication::translate("MainWindow", "TFG", 0));
    resize(800, 600);

    QMenuBar* menubar = new QMenuBar(this);
    menubar->setGeometry(QRect(0, 0, 800, 21));

    QMenu* menu;
    QAction* action;

    MENU("File");
    {
        ACTION("Exit", exit());
    }

    MENU("View");
    {
        ACTION_2("Draw Skeletons", triggered(bool), setDrawSkeletons(bool));
        action->setCheckable(true);
        action->setChecked(true);
    }

    MENU("Actions");
    {
        ACTION("Calibrate", calibrate());
        ACTION("Record", openRecorder());
    }

    MENU("Streams");
    {
        ACTION("Stream Manager", openStreamManager());

        menu->addSeparator();

        ACTION("Kinect 1", openKinect1());
        ACTION("Kinect 2", openKinect2());

        menu->addSeparator();

        ACTION("Recorded Stream...", openRecordedStream());
        ACTION("Fixed Image Stream...", openImageStream());
    }

    menuModes = MENU("Mode");
    {
        ACTION("None", setModeNone());
        ACTION("Measure", setModeMeasure());
    }

    setMenuBar(menubar);

    statusbar = new QStatusBar(this);
    setStatusBar(statusbar);

    //QToolBar* toolBar = new QToolBar(this);
    //toolBar->addSeparator();
    //addToolBar(Qt::TopToolBarArea, toolBar);

    QWidget* centralwidget = new QWidget(this);
    QGridLayout* gridLayout = new QGridLayout(centralwidget);
    gridLayout->setContentsMargins(0, 0, 0, 0);

    mdiArea = new QMdiArea(centralwidget);
    gridLayout->addWidget(mdiArea, 0, 0, 1, 1);

    setCentralWidget(centralwidget);
    
    QObject::connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(changedSubwindow(QMdiSubWindow*)));

    //QMetaObject::connectSlotsByName(this);
}

#pragma endregion
