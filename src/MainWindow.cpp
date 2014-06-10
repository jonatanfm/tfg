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

#include "otger/DepthCorrector.h"

#include "jonatan/CaptureSkeleton.h"
#include "jonatan/SkeletonStudy.h"

#ifdef HAS_LIBXL

#include "libxl.h"
using namespace libxl;

#endif

static MainWindow* instance = nullptr;

// Called when Ptr<DataStream> is deleting its content
void DataStream::deleting(DataStream* obj)
{
    if (instance != nullptr) instance->closeStream(obj);
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
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
        if (streams[i] != nullptr)
        {
            #ifdef HAS_BULLET
                streams[i]->removeNewFrameCallback(&world);
            #endif
            streams[i].addref();
        }
    }

    if (instance == this) instance = nullptr;
}

void MainWindow::initialize()
{
    std::string lineStr, str;
    std::ifstream file("config.txt");
    if (file) {
        while (file && std::getline(file, lineStr)) {
            std::istringstream line(lineStr);
            if (line >> str) {
                if (str.empty() || str[0] == '#') continue;
                if (str == "stream") {
                    if (line >> str) {
                        if (str == "fixed") {
                            std::string sColor, sDepth, sSkeleton;
                            line >> sColor >> sDepth >> sSkeleton;
                            addStream(new FixedFrameStream(sColor, sDepth, sSkeleton));
                        }
                    }
                }
                else if (str == "kinects") {
                    for (int i = 0; i < kinectManager.getSensorCount(); ++i) {
                        openKinect(i);
                    }
                }
                else if (str == "sceneview") openSceneView();
                #ifdef HAS_BULLET
                    else if (str == "augmentedview") openAugmentedView();
                #endif
            }
        }
    }
    else { // No config file, just open connected kinects
        for (int i = 0; i < kinectManager.getSensorCount(); ++i) {
            openKinect(i);
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

void MainWindow::toggleSkeletonsOverlay(WidgetOpenGL* widget)
{
    if (widget == nullptr) return;

    if (widget->hasOverlay("skeleton")) {
        widget->removeOverlay("skeleton");
    }
    else {
        if (widget->is<WidgetColorView>()) {
            widget->addOverlay("skeleton", [](WidgetOpenGL* w) -> bool {
                SkeletonFrame skeleton;
                if (((WidgetColorView*)w)->getStream()->getSkeletonFrame(skeleton)) {
                    RenderUtils::drawSkeletons(skeleton.frame, true);
                }
                return true;
            });
        }
        else if (widget->is<WidgetDepthView>()) {
            widget->addOverlay("skeleton", [](WidgetOpenGL* w) -> bool {
                SkeletonFrame skeleton;
                if (((WidgetDepthView*)w)->getStream()->getSkeletonFrame(skeleton)) {
                    RenderUtils::drawSkeletons(skeleton.frame, false);
                }
                return true;
            });
        }
    }
    changedSubwindow(mdiArea->currentSubWindow());
}

void MainWindow::setDrawSkeletons(bool draw)
{
    QMdiSubWindow* win = mdiArea->currentSubWindow();
    if (win == nullptr) return;

    toggleSkeletonsOverlay(dynamic_cast<WidgetOpenGL*>(win->widget()));
}

void MainWindow::setSmoothSkeletons(bool smooth)
{
    Ptr<DataStream> stream = getCurrentStream();
    if (stream != nullptr) {
        stream->setSkeletonSmoothing(smooth);
        updateToolbar();
    }
}

void MainWindow::setDrawTrajectory()
{
    Ptr<DataStream> stream = getCurrentStream();
    WidgetSceneView* sceneView = findSubwindowByType<WidgetSceneView>();
    if (stream != nullptr && sceneView != nullptr) {
        RecordedStream* rs = dynamic_cast<RecordedStream*>(stream.obj);
        if (rs != nullptr) {
            bool ok;
            QStringList items;
            items << "(Finish)" << "Hip Center" << "Spine" << "Shoulder Center" << "Head" << "Shoulder Left" << "Elbow Left" << "Wrist Left" << "Hand Left" << "Shoulder Right" << "Elbow Right" << "Wrist Right" << "Hand Right" << "Hip Left" << "Knee Left" << "Ankle Left" << "Foot Left" << "Hip Right" << "Knee Right" << "Ankle Right" << "Foot Right";
            
            std::vector<NUI_SKELETON_POSITION_INDEX> joints;
            
            while (true) {
                QString item = QInputDialog::getItem(this, "Select Joints", "Add a joint:", items, 0, false, &ok);
                if (!ok || item.isEmpty()) break;
                int i;
                for (i = 0; i < items.size(); ++i) {
                    if (items[i] == item) break;
                }
                if (i < items.size()) {
                    if (i == 0) break;
                    else {
                        size_t j;
                        for (j = 0; j < joints.size(); ++j) {
                            if (joints[j] == i - 1) break;
                        }
                        if (j == joints.size()) joints.push_back((NUI_SKELETON_POSITION_INDEX) (i - 1));
                    }
                }
            }
            
            SkeletonTrajectory* traj = new SkeletonTrajectory();
            if (rs->getSkeletonTrajectory(joints, *traj)) sceneView->addTrajectory(traj);
            else delete traj;
        }
    }
}

void MainWindow::changedSubwindow(QMdiSubWindow* win)
{
    updateToolbar();

    // Widget specific controls
    QList<QAction*> actions = toolbar->actions();
    for (int i = actions.size() - 1; actions[i] != actionRestart; --i) {
        toolbar->removeAction(actions[i]);
    }

    if (win != nullptr) {
        SubWindowWidget* w = dynamic_cast<SubWindowWidget*>(win->widget());
        if (w != nullptr) {
            int n = toolbar->actions().size();
            w->createActions(toolbar);
            if (n < toolbar->actions().size()) toolbar->insertSeparator(toolbar->actions()[n]);
        }
    }
}

void MainWindow::updateToolbar()
{
    QMdiSubWindow* win = mdiArea->currentSubWindow();
    Ptr<DataStream> stream = getCurrentStream();

    // Draw skeletons
    WidgetOpenGL* w = (win == nullptr) ? nullptr : dynamic_cast<WidgetOpenGL*>(win->widget());
    if (w != nullptr && (w->is<WidgetColorView>() || w->is<WidgetDepthView>())) {
        actionDrawSkeleton->setEnabled(true);
        actionDrawSkeleton->setChecked(w->hasOverlay("skeleton"));
    }
    else actionDrawSkeleton->setEnabled(false);


    // Smooth skeletons
    if (stream != nullptr && stream->hasSkeleton()) {
        actionSmoothSkeleton->setEnabled(true);
        actionSmoothSkeleton->setChecked(stream->getSkeletonSmoothing());
    }
    else actionSmoothSkeleton->setEnabled(false);

    // Recorded stream controls
    RecordedStream* rs = dynamic_cast<RecordedStream*>(stream.obj);
    bool isRecordedStream = (rs != nullptr);

    actionPlayPause->setEnabled(isRecordedStream);
    actionRestart->setEnabled(isRecordedStream);
    actionAdvance->setEnabled(isRecordedStream);

    if (isRecordedStream && !rs->isPaused()) {
        actionPlayPause->setIcon(iconPause);
        actionPlayPause->setText(QApplication::translate("MainWindow", "Pause", 0));
    }
    else {
        actionPlayPause->setIcon(iconPlay);
        actionPlayPause->setText(QApplication::translate("MainWindow", "Play", 0));
    }
}

void MainWindow::streamPlayPause()
{
    RecordedStream* rs = dynamic_cast<RecordedStream*>(getCurrentStream().obj);
    if (rs != nullptr) {
        rs->setPaused(!rs->isPaused());
        updateToolbar();
    }
}

void MainWindow::streamRestart()
{
    RecordedStream* rs = dynamic_cast<RecordedStream*>(getCurrentStream().obj);
    if (rs != nullptr) {
        rs->reset();
        updateToolbar();
    }
}

void MainWindow::streamAdvance()
{
    RecordedStream* rs = dynamic_cast<RecordedStream*>(getCurrentStream().obj);
    if (rs != nullptr) {
        if (!rs->isPaused()) rs->setPaused(true);
        rs->advance();
        updateToolbar();
    }
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
        if (streams[i] == stream) { // Already present
            openStreamWindows(i);
            return i;
        }
        if (idx == -1 && streams[i] == nullptr) idx = i;
    }

    if (idx != -1) streams[idx] = stream;
    else {
        idx = int(streams.size());
        streams.push_back(stream);
    }

    #ifdef HAS_BULLET
        if (idx == 0 && stream->hasSkeleton()) {
            stream.obj->addNewFrameCallback(&world, [this](const ColorFrame* color, const DepthFrame* depth, const SkeletonFrame* skeleton) -> void {
                this->getWorld().setSkeleton(skeleton);
            });
        }
    #endif

    openStreamWindows(idx);

    *stream.refcount -= 1; // Convert the reference in the "streams" vector to a weak reference

    updateStreamManager();

    return idx;
}

void MainWindow::openStreamWindows(int i)
{
    if (i < 0 || i >= int(streams.size()) || streams[i] == nullptr) return;

    Ptr<DataStream> stream = streams[i];

    QString name = QString::fromStdString(stream->getName());

    bool atLeastOne = false;

    if (stream->hasColor())
    {
        WidgetColorView* view = new WidgetColorView(*this, stream);
        addSubWindow(view, "#" + QString::number(i) + " - " + name + " - Color");
        toggleSkeletonsOverlay(view);
        atLeastOne = true;
    }

    if (stream->hasDepth())
    {
        WidgetDepthView* view = new WidgetDepthView(*this, stream);
        addSubWindow(view, "#" + QString::number(i) + " - " + name + " - Depth");
        toggleSkeletonsOverlay(view);
        atLeastOne = true;
    }

    if (!atLeastOne) {
        EmptyView* view = new EmptyView(*this, stream);
        addSubWindow(view, "#" + QString::number(i) + " - " + name);
    }
}

void MainWindow::openKinect(int i)
{
    if (kinectManager.getSensorCount() <= i) return;

    addStream(kinectManager.getStream(i));
}

void MainWindow::openRecorder()
{
    reopenSingletonSubwindow<WidgetRecorder>("Recorder");
}

void MainWindow::openSceneView()
{
    reopenSingletonSubwindow<WidgetSceneView>("Scene View");
}

void MainWindow::openChessboardFinder()
{
    Ptr<DataStream> stream = getCurrentStream();
    if (stream != nullptr) {
        QString sizes = QInputDialog::getText(this, "Find Chessboard", "Input the chessboard size:\n(Number of rows/cols - 1)", QLineEdit::Normal, "6x6");
        auto parts = sizes.trimmed().split(QRegularExpression("\\D"));
        if (parts.size() == 2) {
            addStream(new ChessboardDetectorStream(stream, parts[0].toInt(), parts[1].toInt()));
        }
    }
}

void MainWindow::openDepthCorrector()
{
    Ptr<DataStream> stream = getCurrentStream();
    if (stream != nullptr && stream->hasDepth()) {
        addStream(new DepthCorrectorStream(stream));
    }
}



void MainWindow::openStreamManager()
{
    WidgetStreamManager* w = findSubwindowByType<WidgetStreamManager>();
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
    WidgetStreamManager* w = findSubwindowByType<WidgetStreamManager>();
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
    filters << "Image files (*.png *.bmp *.xpm *.jpg *.skel *.bin)" << "Any files (*)";
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
    dialog.setNameFilter("Stream Videos (*.avi *.wmv *.mp4 *.skel *.bin)");
    if (dialog.exec()) {
        QString color, depth, skeleton;
        if (determineStreamFiles(dialog.selectedFiles(), OUT color, OUT depth, OUT skeleton)) {
            addStream(new RecordedStream(color.toStdString(), depth.toStdString(), skeleton.toStdString()));
        }
    }
}

void MainWindow::setStatusText(QString text)
{
    statusBar->showMessage(text);
}

void MainWindow::setOperationStatus(QString text)
{
    statusBarText->setText(text);
}

void MainWindow::setOperationProgress(int progress, int max)
{
    statusBarProgress->setMaximum(max);
    statusBarProgress->setValue(progress);
}

void MainWindow::operationFinished()
{
    setOperationStatus("Finished");
    setOperationProgress(0, 1);
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
                    calibration.add(i, j, params[k - 1]);
                    qDebug() << "Calibrated streams " << i << " and " << j;
                    qDebug() << QString::fromStdString("R: " + Utils::matToString<double>(params[k - 1].R));
                    qDebug() << QString::fromStdString("T: " + Utils::matToString<double>(params[k - 1].T));
                }
            }
        }
    }

    currentOperation = nullptr;
}

void MainWindow::skeletonTraking()
{
    Ptr<DataStream> stream = getCurrentStream();
    if (stream != nullptr) {
        double d = QInputDialog::getDouble(this, tr("Enter Radius Bone Length"),
                                    tr("Enter Radius Bone Length in cms:\n(with 0 value this info will be ignored)"), 0, 0, 1000, 2);
        double d2 = QInputDialog::getDouble(this, tr("Enter Tibia Bone Length"),
                                    tr("Enter Tibia Bone Length in cms:\n(with 0 value this info will be ignored)"), 0, 0, 1000, 2);
       
        stream->setSkeletonSmoothing(false);
        updateToolbar();

        addStream(new CaptureSkeleton(stream, (float)d, (float)d2));
    }
}

void MainWindow::skeletonWorking()
{
    int type = QInputDialog::getInt(this, tr("Enter mode to improve skeleton:"),
                                        tr("Enter mode to improve skeleton:\nWithout improvement: 0\nWith length check: 1\nWith recover losed data: 2"), 0, 0, 5);

	
		if(type==1){
			QFileDialog dialog(this);
			dialog.setAcceptMode(QFileDialog::AcceptOpen);
			dialog.setFileMode(QFileDialog::ExistingFiles);
			dialog.setNameFilter("Skeleton tracked data (*.xls *.xlxs)");
			if (dialog.exec()) {
				QString tmp = dialog.selectedUrls().at(0).toEncoded();
				QString delimiterPattern("///");
				QStringList mailids = tmp.split(delimiterPattern);
				tmp=mailids[1];

				SubWindowWidget* w = dynamic_cast<SubWindowWidget*>(mdiArea->currentSubWindow()->widget());
				if (w != nullptr) {
					Ptr<DataStream> stream = w->getStream();
					if (stream != nullptr) {

						 addStream(new SkeletonStudy(stream,type,tmp));
            
					}
				}
			}
		}
		else{
			SubWindowWidget* w = dynamic_cast<SubWindowWidget*>(mdiArea->currentSubWindow()->widget());
				if (w != nullptr) {
					Ptr<DataStream> stream = w->getStream();
					if (stream != nullptr) {

						 addStream(new SkeletonStudy(stream,type,""));

            
					}
				}
		}
}

void MainWindow::startOperation(Operation* op, std::function< void() > callback)
{
    if (currentOperation != nullptr) {
        delete op;
        return;
    }

    statusBarProgress->setEnabled(true);
    setOperationProgress(0, 0);
    setOperationStatus("Executing...");
    
    connect(op, SIGNAL(statusChanged(QString)), this, SLOT(setOperationStatus(QString)));
    connect(op, SIGNAL(progressChanged(int, int)), this, SLOT(setOperationProgress(int, int)));

    AsyncOperation* asyncOp = new AsyncOperation(op, callback);
    currentOperation = asyncOp;
    connect(asyncOp, SIGNAL(finished()), this, SLOT(operationFinished()));
    asyncOp->start();
}



#ifdef HAS_BULLET

    #include "otger/WidgetAugmentedView.h"

    void MainWindow::openAugmentedView()
    {
        reopenSingletonSubwindow<WidgetAugmentedView>("Augmented View");
    }

#else

    void MainWindow::openAugmentedView() { }

#endif



#pragma region Setup UI
//
// Setup the window UI (the menus)
//


// Some auxiliar defines to simplify the menus creation:


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

#define SHORTCUT(_keys) { action->setShortcut(QKeySequence(_keys)); }

#define ACTION_SHORTCUT(_text, _slot, _keys) { \
        ACTION(_text, _slot); \
        SHORTCUT(_keys); \
    }

#define ACTION_ICON(_text, _slot, _icon) { \
        action = new QAction(QApplication::translate("MainWindow", (_text), 0), this); \
        action->setIcon(QIcon(_icon)); \
        QObject::connect(action, SIGNAL(triggered()), this, SLOT(_slot)); \
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
        ACTION_SHORTCUT("Scene View", openSceneView(), Qt::Key_F3);

        #ifdef HAS_BULLET
            ACTION_SHORTCUT("Augmented View", openAugmentedView(), Qt::Key_F4);
        #endif

        menu->addSeparator();

        ACTION_SHORTCUT("Show Skeleton Trajectories", setDrawTrajectory(), Qt::Key_F6);

        ACTION_2("Smooth Skeletons", triggered(bool), setSmoothSkeletons(bool));
        SHORTCUT(Qt::Key_F7);
        action->setCheckable(true);
        action->setChecked(false);
        actionSmoothSkeleton = action;

        ACTION_2("Draw Skeletons", triggered(bool), setDrawSkeletons(bool));
        SHORTCUT(Qt::Key_F5);
        action->setCheckable(true);
        action->setChecked(false);
        actionDrawSkeleton = action;
    }

    MENU("Actions");
    {
        ACTION("Find chessboards", openChessboardFinder());

        ACTION("Correct depth", openDepthCorrector());

        ACTION("Track Skeleton Lengths", skeletonTraking());

        ACTION("Skeleton Study", skeletonWorking());

        ACTION_SHORTCUT("Record", openRecorder(), Qt::Key_F12);
    }

    MENU("Streams");
    {
        ACTION_SHORTCUT("Stream Manager", openStreamManager(), Qt::Key_F1);

        menu->addSeparator();

        ACTION_SHORTCUT("Kinect 0", openKinect0(), Qt::CTRL + Qt::Key_0);
        ACTION_SHORTCUT("Kinect 1", openKinect1(), Qt::CTRL + Qt::Key_1);

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

    iconPlay = QIcon(":/control_play_blue.png");
    iconPause = QIcon(":/control_pause_blue.png");

    toolbar = new QToolBar(this);
    {
        QToolBar* menu = toolbar;

        ACTION_ICON("Fixed Image Stream...", openImageStream(), ":/folder_picture.png");
        ACTION_ICON("Recorded Stream...", openRecordedStream(), ":/folder_camera.png");

        menu->addSeparator();

        ACTION_ICON("Record", openRecorder(), ":/film_save.png");

        menu->addSeparator();

        ACTION_ICON("", streamPlayPause(), "");
        actionPlayPause = action;

        ACTION_ICON("Advance one frame", streamAdvance(), ":/control_fastforward_blue.png");
        actionAdvance = action;

        ACTION_ICON("Reset", streamRestart(), ":/control_start_blue.png");
        actionRestart = action;
    }
    addToolBar(Qt::TopToolBarArea, toolbar);

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
