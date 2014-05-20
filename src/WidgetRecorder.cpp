#include "WidgetRecorder.h"
#include "ui_WidgetRecorder.h"

#include "MainWindow.h"

#include "SkeletonIO.h"

#include "Utils.h"

#pragma region class Recorder

    class Recorder : private QThread
    {
        public:
            Recorder(Ptr<DataStream> stream) :
                recording(false),
                stream(stream)
            {
            }

            virtual ~Recorder()
            {
                stop();
            }

            bool setup(const QString& filename, int fourcc)
            {
                if (recording) return false;
                return setupWriter(filename, fourcc);
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

        protected:
            Ptr<DataStream> stream;

            volatile bool recording;

            virtual bool setupWriter(const QString& file, int fourcc) = 0;

            virtual void run() = 0;

    };



    /*{
    writer.write(cv::Mat& mat)
    writer.

    cv::VideoWriter makeVideo;
    makeVideo.open("makevideo//newVideo.mp4", CV_FOURCC('X', '2', '6', '4'), 30, cv::Size(1600, 1200), true);
    cv::Mat image = imread("makevideo//frames//111.png");

    for (int i = 0; i < 200; i++)
    makeVideo << image;

    makeVideo.release();
    }*/

#pragma endregion


#pragma region class ColorRecorder

    class ColorRecorder : public Recorder
    {
        public:
            ColorRecorder(Ptr<DataStream> stream) :
                Recorder(stream),
                buffer(cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT), CV_8UC3)
            {

            }

            bool setupWriter(const QString& file, int fourcc) override
            {
                //const int fourcc = CV_FOURCC('L', 'A', 'G', 'S');
                //const int fourcc = CV_FOURCC('X', '2', '6', '4');
                //const int fourcc = CV_FOURCC('Z', 'L', 'I', 'B');
                //const int fourcc = CV_FOURCC('M', 'J', 'P', 'G');
                //const int fourcc = CV_FOURCC_PROMPT;
                //const int fourcc = CV_FOURCC('H', 'F', 'Y', 'U');
                //int fourcc = CV_FOURCC('H', 'F', 'Y', 'U');

                writer.open(
                    (file + ".avi").toStdString(),
                    fourcc,
                    30.0,
                    cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT),
                    true
                );

                return writer.isOpened();
            }

            void run() override
            {
                while (recording) {
                    if (stream->waitForFrame(&frame, nullptr, nullptr)) {
                        Utils::colorFrameToRgb(frame, buffer);
                        writer << buffer;
                    }
                }
                writer.release();
            }

        private:
            cv::VideoWriter writer;
            cv::Mat buffer;

            ColorFrame frame;


    };

#pragma endregion


#pragma region class DepthRecorder

    class DepthRecorder : public Recorder
    {
        public:
            DepthRecorder(Ptr<DataStream> stream) :
                Recorder(stream),
                buffer(cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT), CV_8UC3)
            {

            }

            bool setupWriter(const QString& file, int fourcc) override
            {
                writer.open(
                    (file + ".avi").toStdString(),
                    fourcc,
                    30.0,
                    cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT),
                    true
                );

                return writer.isOpened();
            }

            void run() override
            {
                while (recording) {
                    if (stream->waitForFrame(nullptr, &frame, nullptr)) {
                        Utils::depthFrameToRgb(frame, buffer);
                        writer << buffer;
                    }
                }
                writer.release();
            }

        private:
            cv::VideoWriter writer;
            cv::Mat buffer;

            DepthFrame frame;

    };

#pragma endregion

#pragma region class SkeletonRecorder

    class SkeletonRecorder : public Recorder
    {
        public:
            SkeletonRecorder(Ptr<DataStream> stream) :
                Recorder(stream)
            {
            
            }

            bool setupWriter(const QString& filename, int) override
            {
                QString f = filename;
                //int i = f.lastIndexOf('.');
                //if (i != -1) f = f.mid(0, i);
                f += ".skel";

                return writer.openFileForWriting(f.toUtf8().data());
            }

            void run() override
            {
                while (recording) {
                    if (stream->waitForFrame(nullptr, nullptr, &frame)) {
                        writer.writeFrame(frame);
                    }
                    Sleep(10);
                }

                writer.close();
            }

        private:
            SkeletonFrame frame;

            SkeletonIO writer;

    };

#pragma endregion





WidgetRecorder::WidgetRecorder(MainWindow& mainWindow, QWidget *parent) :
    QWidget(parent),
    mainWindow(mainWindow),
    ui(new Ui::WidgetRecorder),
    recording(false)
{
    ui->setupUi(this);
    ui->txtFilename->setText(QDir::currentPath() + "/capture<TIME>_<ID>_<TYPE>");

    updateStreamList();
}

WidgetRecorder::~WidgetRecorder()
{
    delete ui;
}

void WidgetRecorder::record()
{
    recording = !recording;
    if (recording) {
        QString filenameTemplate = ui->txtFilename->text();

        std::string codec = ui->txtCodecColor->text().toStdString();
        int colorCodec = (codec.size() == 4) ? CV_FOURCC(codec[0], codec[1], codec[2], codec[3]) : CV_FOURCC_PROMPT;

        codec = ui->txtCodecDepth->text().toStdString();
        int depthCodec = (codec.size() == 4) ? CV_FOURCC(codec[0], codec[1], codec[2], codec[3]) : CV_FOURCC_PROMPT;

        QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch() / 1000);

        KinectManager& k = mainWindow.getKinectManager();

        bool allInitialized = true;
        for (int i = 0; i < ui->list->count(); ++i) {
            QListWidgetItem* item = ui->list->item(i);
            if (item->checkState() == Qt::Checked) {
                QString str = item->text();
                int device = str.at(7).toLatin1() - '0' - 1; 

                Ptr<KinectStream> stream = k.getStream(device);

                Recorder* recorder;
                QString type;
                if (str.endsWith("Skeleton")) {
                    recorder = new SkeletonRecorder(stream);
                    type = "skeleton";
                }
                else if (str.endsWith("Depth")) {
                    recorder = new DepthRecorder(stream);
                    type = "depth";
                }
                else {
                    recorder = new ColorRecorder(stream);
                    type = "color";
                }

                QString filename = filenameTemplate;
                filename.replace("<TYPE>", type)
                        .replace("<TIME>", timestamp)
                        .replace("<ID>", QString::number(device));

                if (!recorder->setup(filename, (type == "depth") ? depthCodec : colorCodec))
                {
                    allInitialized = false;
                    break;
                }
                recorders.push_back(recorder);
            }
        }

        if (allInitialized) {
            if (recorders.size() > 0) {
                ui->progressBar->setMaximum(0);
                ui->btnRecord->setText("Stop");

                for (int i = 0; i < int(recorders.size()); ++i) {
                    recorders[i]->begin();
                }
            }
        }
        else {
            recording = false;
            for (int i = 0; i < int(recorders.size()); ++i) {
                recorders[i]->stop();
                delete recorders[i];
            }
            recorders.clear();
            QMessageBox::critical(this, "Recording Error", "Unable to initialize video writer for one or more streams.\nTry changing the Codec and/or the file extension.");
        }
    }
    else {
        for (int i = 0; i < int(recorders.size()); ++i) {
            recorders[i]->stop();
            delete recorders[i];
        }
        recorders.clear();

        ui->progressBar->setMaximum(1);
        ui->btnRecord->setText("Record");
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





