#include "WidgetRecorder.h"
#include "ui_WidgetRecorder.h"

#include "MainWindow.h"

#include "SkeletonIO.h"

#include "Utils.h"

#pragma region Stream Recorder

    // Manages recording over a single stream
    class StreamRecorder
    {
        private:
            std::string colorFile;
            cv::VideoWriter colorWriter;
            cv::Mat colorBuffer;
            ColorFrame* colorFrame;
            
            std::string depthFile;
            cv::VideoWriter depthWriter;
            cv::Mat depthBuffer;
            DepthFrame* depthFrame;
            
            std::string skeletonFile;
            SkeletonIO skeletonWriter;
            SkeletonFrame* skeletonFrame;
            
        public:
            StreamRecorder(Ptr<DataStream> stream) :
                stream(stream),
                colorFrame(nullptr),
                depthFrame(nullptr),
                skeletonFrame(nullptr)
            {

            }

            ~StreamRecorder()
            {
                colorWriter.release();
                depthWriter.release();
                skeletonWriter.close();
                if (colorFrame != nullptr) delete colorFrame;
                if (depthFrame != nullptr) delete depthFrame;
                if (skeletonFrame != nullptr) delete skeletonFrame;
            }
            
            bool recordColor(const QString& file, int fourcc)
            {
                //const int fourcc = CV_FOURCC('L', 'A', 'G', 'S');
                //const int fourcc = CV_FOURCC('X', '2', '6', '4');
                //const int fourcc = CV_FOURCC('Z', 'L', 'I', 'B');
                //const int fourcc = CV_FOURCC('M', 'J', 'P', 'G');
                //const int fourcc = CV_FOURCC_PROMPT;
                //const int fourcc = CV_FOURCC('H', 'F', 'Y', 'U');
                //int fourcc = CV_FOURCC('H', 'F', 'Y', 'U');

                colorFile = (file + ".avi").toStdString();

                colorWriter.open(
                    colorFile,
                    fourcc,
                    30.0,
                    cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT),
                    true
                );

                if (colorWriter.isOpened()) {
                    colorBuffer.create(cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT), CV_8UC3);
                    colorFrame = new ColorFrame();
                    return true;
                }
                colorFile.clear();
                return false;
            }
            
            bool recordDepth(const QString& file, int fourcc)
            {
                depthFile = (file + ".avi").toStdString();

                depthWriter.open(
                    depthFile,
                    fourcc,
                    30.0,
                    cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT),
                    true
                );

                if (depthWriter.isOpened()) {
                    depthBuffer.create(cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT), CV_8UC3);
                    depthFrame = new DepthFrame();
                    return true;
                }
                depthFile.clear();
                return false;
            }
            
            bool recordSkeleton(const QString& file)
            {
                QString f = file;
                //int i = f.lastIndexOf('.');
                //if (i != -1) f = f.mid(0, i);
                skeletonFile = (f + ".skel").toStdString();

                if (skeletonWriter.openFileForWriting(skeletonFile.c_str())) {
                    skeletonFrame = new SkeletonFrame();
                    return true;
                }
                return false;
            }

            void getFiles(OUT std::vector<std::string>& files)
            {
                if (!colorFile.empty()) files.push_back(colorFile);
                if (!depthFile.empty()) files.push_back(depthFile);
                if (!skeletonFile.empty()) files.push_back(skeletonFile);
            }
            
            void grabFrames()
            {
                stream->waitForFrame(colorFrame, depthFrame, skeletonFrame);
            }
            
            void storeFrames()
            {
                if (colorFrame != nullptr) {
                    Utils::colorFrameToRgb(*colorFrame, colorBuffer);
                    colorWriter << colorBuffer;
                }

                if (depthFrame != nullptr) {
                    Utils::depthFrameToRgb(*depthFrame, depthBuffer);
                    depthWriter << depthBuffer;
                }

                if (skeletonFrame != nullptr) {
                    skeletonWriter.writeFrame(*skeletonFrame);
                }
            }

        protected:
            Ptr<DataStream> stream;

    };

    //{
    //writer.write(cv::Mat& mat)
    //writer.

    //cv::VideoWriter makeVideo;
    //makeVideo.open("makevideo//newVideo.mp4", CV_FOURCC('X', '2', '6', '4'), 30, cv::Size(1600, 1200), true);
    //cv::Mat image = imread("makevideo//frames//111.png");

    //for (int i = 0; i < 200; i++)
    //makeVideo << image;

    //makeVideo.release();
    //}

#pragma endregion


#pragma region Recorder

    // Manages asynchronous recording for multiple streams
    class Recorder : private QThread
    {
        private:
            std::vector<StreamRecorder*> recorders;

        public:
            Recorder() :
                recording(false)
            {

            }

            ~Recorder()
            {
                for (size_t i = 0; i < recorders.size(); ++i) delete recorders[i];
            }
            
            void add(StreamRecorder* recorder)
            {
                recorders.push_back(recorder);
            }
            
            void begin()
            {
                if (!recording) {
                    recording = true;
                    this->start();
                }
            }

            void stop()
            {
                if (recording) {
                    recording = false;
                    this->wait();
                }
            }

            void getFiles(OUT std::vector<std::string>& files)
            {
                for (size_t i = 0; i < recorders.size(); ++i) {
                    recorders[i]->getFiles(files);
                }
            }

        protected:
            volatile bool recording;

            void run() override
            {
                while (recording) {
                    for (size_t i = 0; i < recorders.size(); ++i) {
                        recorders[i]->grabFrames();
                    }
                    for (size_t i = 0; i < recorders.size(); ++i) {
                        recorders[i]->storeFrames();
                    }
                }
            }

    };

#pragma endregion


WidgetRecorder::WidgetRecorder(MainWindow& mainWindow, QWidget *parent) :
    QWidget(parent),
    mainWindow(mainWindow),
    ui(new Ui::WidgetRecorder),
    recorder(nullptr)
{
    ui->setupUi(this);
    ui->txtFilename->setText(QDir::currentPath() + "/capture<TIME>_<ID>_<TYPE>");

    updateStreamList();
}

WidgetRecorder::~WidgetRecorder()
{
    delete ui;
}

struct RecordTargets
{
    Ptr<DataStream> stream;
    int index;
    bool color, depth, skeleton;
    
    RecordTargets() : stream(nullptr), color(false), depth(false), skeleton(false) { }
};

std::string fixPath(const std::string& path)
{
    std::string str = path;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '/') str[i] = '\\';
    }
    return str;
}

void showFilesInExplorer(const std::vector<std::string>& files)
{
    size_t n = files.size();
    if (n == 0) return;

    std::string folder = fixPath(files[0]);
    size_t i = folder.find_last_of('\\');
    if (i != std::string::npos) folder = folder.substr(0, i);

    #ifdef _WIN32
        ITEMIDLIST* dir = ILCreateFromPathA(folder.c_str());
        LPITEMIDLIST* items = new LPITEMIDLIST[n];
        for (size_t i = 0; i < n; ++i) items[i] = ILCreateFromPathA(fixPath(files[i]).c_str());

        SHOpenFolderAndSelectItems(dir, (unsigned int)n, (LPCITEMIDLIST*)items, 0);

        for (size_t i = 0; i < n; ++i) ILFree(items[i]);
        ILFree(dir);
        delete[] items;
    #endif
}

void WidgetRecorder::record()
{
    if (recorder == nullptr) {
        QString filenameTemplate = ui->txtFilename->text();

        std::string codec = ui->txtCodecColor->text().toStdString();
        qDebug() << "Codec size " << codec.size();
        int colorCodec = (codec.size() == 4) ? CV_FOURCC(codec[0], codec[1], codec[2], codec[3]) : CV_FOURCC_PROMPT;

        codec = ui->txtCodecDepth->text().toStdString();
        int depthCodec = (codec.size() == 4) ? CV_FOURCC(codec[0], codec[1], codec[2], codec[3]) : CV_FOURCC_PROMPT;

        QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);

        KinectManager& k = mainWindow.getKinectManager();

        std::vector<RecordTargets> recordTargets;
        
        for (int i = 0; i < ui->list->count(); ++i) {
            QListWidgetItem* item = ui->list->item(i);
            if (item->checkState() == Qt::Checked) {
                QString str = item->text();
                int device = str.at(7).toLatin1() - '0' - 1; 

                Ptr<KinectStream> stream = k.getStream(device);

                if (stream != nullptr) {
                    size_t j;
                    for (j = 0; j < recordTargets.size(); ++j) {
                        if (recordTargets[j].stream == stream) break;
                    }
                    if (j == recordTargets.size()) {
                        recordTargets.push_back(RecordTargets());
                        recordTargets[j].stream = stream;
                        recordTargets[j].index = device;
                    }

                    if (str.endsWith("Skeleton")) recordTargets[j].skeleton = true;
                    else if (str.endsWith("Depth")) recordTargets[j].depth = true;
                    else recordTargets[j].color = true;
                }
            }
        }
        
        bool ok = true;
        
        Recorder* newRecorder = new Recorder();
        for (size_t i = 0; i < recordTargets.size() && ok; ++i) {
            if (recordTargets[i].color || recordTargets[i].depth || recordTargets[i].skeleton) {
                StreamRecorder* rec = new StreamRecorder(recordTargets[i].stream);
                
                QString filename = filenameTemplate
                    .replace("<TIME>", timestamp)
                    .replace("<ID>", QString::number(recordTargets[i].index));

                if (recordTargets[i].color && ok) {
                    QString file = filename;
                    ok = ok && rec->recordColor(file.replace("<TYPE>", "color"), colorCodec);
                }
                if (recordTargets[i].depth && ok) {
                    QString file = filename;
                    ok = ok && rec->recordDepth(file.replace("<TYPE>", "depth"), depthCodec);
                }
                if (recordTargets[i].skeleton && ok) {
                    QString file = filename;
                    ok = ok && rec->recordSkeleton(file.replace("<TYPE>", "skeleton"));
                }
                
                newRecorder->add(rec);
            }
        }

        if (ok) {
            ui->progressBar->setMaximum(0);
            ui->btnRecord->setText("Stop");

            recorder = newRecorder;
            recorder->begin();
        }
        else {
            delete newRecorder;
            QMessageBox::critical(this, "Recording Error", "Unable to initialize video writer for one or more streams.\nTry changing the Codec and/or the file extension.");
        }
    }
    else {
        recorder->stop();

        std::vector<std::string> files;
        recorder->getFiles(files);

        delete recorder;
        recorder = nullptr;

        ui->progressBar->setMaximum(1);
        ui->btnRecord->setText("Record");

        showFilesInExplorer(files);
    }
}

void WidgetRecorder::captureFrame()
{
    QString filenameTemplate = ui->txtFilename->text();
    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);

    KinectManager& k = mainWindow.getKinectManager();

    for (int i = 0; i < ui->list->count(); ++i) {
        QListWidgetItem* item = ui->list->item(i);
        if (item->checkState() == Qt::Checked) {
            QString str = item->text();
            int device = str.at(7).toLatin1() - '0' - 1;

            Ptr<KinectStream> stream = k.getStream(device);

            QString type;
            if (str.endsWith("Skeleton")) type = "skeleton";
            else if (str.endsWith("Depth")) type = "depth";
            else type = "color";

            QString filename = filenameTemplate;
            filename.replace("<TYPE>", type)
                .replace("<TIME>", timestamp)
                .replace("<ID>", QString::number(device));

            if (type == "skeleton") captureSkeletonFrame(stream, filename);
            else if (type == "depth") captureDepthFrame(stream, filename);
            else captureColorFrame(stream, filename);
        }
    }

}

void WidgetRecorder::captureColorFrame(Ptr<DataStream> stream, QString filename)
{
    ColorFrame* frame = new ColorFrame();
    stream->getColorFrame(*frame);

    cv::Mat img(cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT), CV_8UC3);
    Utils::colorFrameToRgb(*frame, img);
    cv::imwrite((filename + ".png").toStdString(), img);

    delete frame;
}

void WidgetRecorder::captureDepthFrame(Ptr<DataStream> stream, QString filename)
{
    DepthFrame* frame = new DepthFrame();
    stream->getDepthFrame(*frame);

    cv::Mat img(cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT), CV_8UC3);
    Utils::depthFrameToRgb(*frame, img);
    cv::imwrite((filename + ".png").toStdString(), img);

    delete frame;
}

void WidgetRecorder::captureSkeletonFrame(Ptr<DataStream> stream, QString filename)
{
    SkeletonFrame frame;
    stream->getSkeletonFrame(frame);

    SkeletonIO writer;
    if (!writer.openFileForWriting((filename + ".skel").toStdString().c_str())) return;
    writer.writeFrame(frame);
    writer.close();
}


void addListItem(QListWidget* list, const QString& text)
{
    QListWidgetItem* item = new QListWidgetItem();
    item->setText(text);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    list->addItem(item);
}

void WidgetRecorder::updateStreamList()
{
    ui->list->clear();

    KinectManager& k = mainWindow.getKinectManager();
    for (int i = 0; i < k.getSensorCount(); ++i) if (k.getStream(i) != nullptr) {
        addListItem(ui->list, "Kinect " + QString::number(i + 1) + " - Color");
        addListItem(ui->list, "Kinect " + QString::number(i + 1) + " - Depth");
        addListItem(ui->list, "Kinect " + QString::number(i + 1) + " - Skeleton");
    }
}





