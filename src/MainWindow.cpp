#include "MainWindow.h"

#include "WidgetColorView.h"
#include "WidgetDepthView.h"
#include "WidgetSceneView.h"
#include "WidgetRecorder.h"
#include "WidgetStreamManager.h"

#include "FixedFrameStream.h"
#include "RecordedStream.h"
#include "ChessboardDetectorStream.h"

#include "Calibrator.h"
#include "Operation.h"

#include "ModeMeasure.h"



static MainWindow* instance = nullptr;

// Called when Ptr<DataStream> is deleting its content
void DataStream::deleting(DataStream* obj)
{
    if (instance != nullptr)
    {
        qDebug() << "Closing Stream " << obj;
        instance->closeStream(obj);
    }
    qDebug() << "Deleting Stream " << obj;
}



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
    instance = this;

    setupUi();

    setModeNone();

    initialize();
}

MainWindow::~MainWindow()
{
    // Increment references to the streams, as they will be decreased
    // when they are released, but they are meant to be weak references
    for (int i = 0; i < int(streams.size()); ++i) {
        if (streams[i] != nullptr) streams[i].addref();
    }

    if (instance == this) instance = nullptr;
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
                        addStream(new FixedFrameStream(sColor, sDepth, sSkeleton));
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


void MainWindow::addSubWindow(SubWindowWidget* widget, const QString& title)
{
    SubWindow* win = new SubWindow(this);
    win->setWidget(dynamic_cast<QWidget*>(widget));
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


int MainWindow::findStreamIndex(const Ptr<DataStream>& stream)
{
    for (int i = 0; i < int(streams.size()); ++i) {
        if (streams[i] == stream) return i;
    }
    return -1;
}

bool MainWindow::closeStream(int i)
{
    if (i >= 0 && i < int(streams.size())) {
        streams[i] = nullptr;
        updateStreamManager();
        return true;
    }
    return false;
}


bool MainWindow::closeStream(DataStream* stream)
{
    for (int i = 0; i < int(streams.size()); ++i) {
        if (streams[i] == stream) {
            streams[i] = nullptr;
            updateStreamManager();
            return true;
        }
    }
    return false;
}

int MainWindow::addStream(const Ptr<DataStream>& stream)
{
    if (stream == nullptr) return -1;

    int idx = -1;
    for (int i = 0; i < int(streams.size()); ++i) {
        if (streams[i] == stream) return i;
        if (idx == -1 && streams[i] == nullptr) idx = i;
    }

    if (idx != -1) streams[idx] = stream;
    else {
        idx = int(streams.size());
        streams.push_back(stream);
    }

    openStreamWindows(idx);

    *stream.refcount -= 1; // Convert the reference in "streams" to a weak reference

    updateStreamManager();

    return idx;
}

void MainWindow::openStreamWindows(int i)
{
    if (i >= int(streams.size())) return;

    Ptr<DataStream> stream = streams[i];

    QString name = QString::fromStdString(stream->getName());

    if (stream->hasColor())
    {
        WidgetColorView* colorView = new WidgetColorView(*this, stream);
        addSubWindow(colorView, "#" + QString::number(i) + " - " + name + " - Color");
    }

    if (stream->hasDepth())
    {
        WidgetDepthView* depthView = new WidgetDepthView(*this, stream);
        addSubWindow(depthView, "#" + QString::number(i) + " - " + name + " - Depth");
    }
}

void MainWindow::openKinect(int i)
{
    if (kinectManager.getSensorCount() <= i) return;

    addStream(kinectManager.getStream(i));
}

void MainWindow::openRecorder()
{
    WidgetRecorder* w = findSubwindowByType<WidgetRecorder>(mdiArea);
    if (w == nullptr) addSubWindow(new WidgetRecorder(*this), "Recorder");
}

void MainWindow::openSceneView()
{
    WidgetSceneView* w = findSubwindowByType<WidgetSceneView>(mdiArea);
    if (w == nullptr) addSubWindow(new WidgetSceneView(*this), "Scene View");
}

void MainWindow::openChessboardFinder()
{
    SubWindowWidget* w = dynamic_cast<SubWindowWidget*>(mdiArea->currentSubWindow()->widget());
    if (w != nullptr) {
        Ptr<DataStream> stream = w->getStream();
        if (stream != nullptr) {
            QString sizes = QInputDialog::getText(this, "Find Chessboard", "Input the chessboard size:\n(Number of rows/cols - 1)", QLineEdit::Normal, "6x6");
            auto parts = sizes.trimmed().split(QRegularExpression("\\D"));
            if (parts.size() == 2) {
                addStream(new ChessboardDetectorStream(stream, parts[0].toInt(), parts[1].toInt()));
            }
        }
    }
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
        addSubWindow(w, "Stream Manager");
    }
}

void MainWindow::updateStreamManager()
{
    WidgetStreamManager* w = findSubwindowByType<WidgetStreamManager>(mdiArea);
    if (w != nullptr) w->refresh();
}

static bool determineStreamFiles(const QStringList& list, OUT QString& color, OUT QString& depth, OUT QString& skeleton)
{
    color = ""; depth = ""; skeleton = "";
    for (int i = 0; i < list.size(); ++i) {
        if (list[i].indexOf("skeleton") != -1) skeleton = list[i];
        else if (list[i].indexOf("depth") != -1) depth = list[i];
        else color = list[i];
    }
    return !color.isEmpty() || !depth.isEmpty() || !skeleton.isEmpty();
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
        QString color, depth, skeleton;
        if (determineStreamFiles(dialog.selectedFiles(), OUT color, OUT depth, OUT skeleton)) {
            addStream(new FixedFrameStream(color.toStdString(), depth.toStdString(), skeleton.toStdString()));
        }
    }
}

void MainWindow::openRecordedStream()
{
    QFileDialog dialog(this);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Stream Videos (*.avi *.wmv *.mp4 *.bin)");
    if (dialog.exec()) {
        QString color, depth, skeleton;
        if (determineStreamFiles(dialog.selectedFiles(), OUT color, OUT depth, OUT skeleton)) {
            addStream(new RecordedStream(color.toStdString(), depth.toStdString(), skeleton.toStdString()));
        }
    }
}

void MainWindow::setStatusText(QString text)
{
    //statusBar->showMessage(text);
    statusBarText->setText(text);
}

void MainWindow::setStatusProgress(int progress, int max)
{
    statusBarProgress->setMaximum(max);
    statusBarProgress->setValue(progress);
}

void MainWindow::operationFinished()
{
    setStatusText("Finished");
    setStatusProgress(0, 1);
    statusBarProgress->setEnabled(false);

    Operation* op = currentOperation->getOperation();
    
    Calibrator* calib = dynamic_cast<Calibrator*>(op);
    if (calib != nullptr && calib->hasExtrinsicParams()) { // Was a calibration?
        auto& streams = calib->getStreams();
        auto& params = calib->getExtrinsicParams();
        int i = findStreamIndex(streams[0]);
        if (i != -1) {
            for (size_t k = 1; k < streams.size(); ++k) {
                int j = findStreamIndex(streams[k]);
                if (j != -1) {
                    calibration.add(i, j, params[j - 1]);
                    qDebug() << "Calibrated streams " << i << " and " << j;
                    qDebug() << QString::fromStdString("R: " + Utils::matToString<double>(params[j - 1].R));
                    qDebug() << QString::fromStdString("T: " + Utils::matToString<double>(params[j - 1].T));
                }
            }
        }
    }

    currentOperation = nullptr;
}


void MainWindow::startOperation(Operation* op, std::function< void() > callback)
{
    if (currentOperation != nullptr) {
        delete op;
        return;
    }

    statusBarProgress->setEnabled(true);
    setStatusProgress(0, 0);
    setStatusText("Executing...");
    
    connect(op, SIGNAL(statusChanged(QString)), this, SLOT(setStatusText(QString)));
    connect(op, SIGNAL(progressChanged(int, int)), this, SLOT(setStatusProgress(int, int)));

    AsyncOperation* asyncOp = new AsyncOperation(op, callback);
    currentOperation = asyncOp;
    connect(asyncOp, SIGNAL(finished()), this, SLOT(operationFinished()));
    asyncOp->start();
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
        ACTION("Scene View", openSceneView());

        menu->addSeparator();

        ACTION_2("Toggle Draw Skeletons", triggered(bool), setDrawSkeletons(bool));
        //action->setCheckable(true);
        //action->setChecked(true);
    }

    MENU("Actions");
    {
        ACTION("Find chessboards", openChessboardFinder());

        ACTION("Record", openRecorder());
    }

    MENU("Streams");
    {
        ACTION("Stream Manager", openStreamManager());

        menu->addSeparator();

        ACTION("Kinect 1", openKinect1());
        ACTION("Kinect 2", openKinect2());

        menu->addSeparator();

        ACTION("Fixed Image Stream...", openImageStream());
        ACTION("Recorded Stream...", openRecordedStream());
    }

    menuModes = MENU("Mode");
    {
        ACTION("None", setModeNone());
        ACTION("Measure", setModeMeasure());
    }

    setMenuBar(menubar);

    statusBar = new QStatusBar(this);
    statusBarText = new QLabel();
    statusBarProgress = new QProgressBar();
    statusBarProgress->setTextVisible(true);
    statusBarProgress->setFixedSize(QSize(192, 16));
    statusBarProgress->setDisabled(true);
    statusBar->addPermanentWidget(statusBarText);
    statusBar->addPermanentWidget(statusBarProgress);
    setStatusBar(statusBar);

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
