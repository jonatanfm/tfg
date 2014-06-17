
#include <QApplication>

#include "MainWindow.h"


// Function type for the main functions
typedef int(*MainFunction)(QApplication&);


// OpenCV Video Test (to try availability and performance of different codecs)
/*int videoTest(QApplication& app)
{
    cv::Mat original = cv::imread("D:/Projects/TFG/bin/capture1399439630_1_depth.png");

    DataStream::DepthPixel* frame = DataStream::newDepthFrame();

    Utils::rgbToDepthFrame(original, frame);

    cv::Mat mat(cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT), CV_8UC3);

    Utils::depthFrameToRgb(frame, mat);

    cv::VideoWriter writer;
    writer.open(
        "TEST.avi",
        //CV_FOURCC('H', 'F', 'Y', 'U'),
        CV_FOURCC('D', 'I', 'B', ' '),
        30.0,
        cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT),
        true
    );

    bool open = writer.isOpened();

    writer << mat;
    writer.release();

    cv::VideoCapture capture;
    capture.open("TEST.avi");

    cv::Mat mat2;
    capture >> mat2;
    capture.release();


    DataStream::DepthPixel* frame2 = DataStream::newDepthFrame();

    Utils::rgbToDepthFrame(mat2, frame2);

    int cmp = memcmp(frame, frame2, DepthFrame::WIDTH * DepthFrame::HEIGHT * sizeof(DataStream::DepthPixel));

    bool areEqual = (cmp == 0);
    return areEqual;
}*/


#ifdef HAS_BULLET
    #include "otger/WidgetAugmentedView.h"
#endif

// Only augmented reality view
int mainAugmented(QApplication& app)
{
    int returnValue = 0;

    #ifdef HAS_BULLET
        KinectStream* stream = new KinectStream();
        if (stream->initialize(0)) {

            World* world = new World();

            stream->addNewFrameCallback(world, [world](const ColorFrame* color, const DepthFrame* depth, const SkeletonFrame* skeleton) -> void {
                world->setSkeleton(skeleton);
            });

            QMainWindow* window = new QMainWindow();
            window->setWindowTitle("TFG");
            window->resize(960, 720);
            window->setCentralWidget(new WidgetOpenGL(new WidgetAugmentedView(stream, *world)));
            window->show();

            returnValue = app.exec();

            stream->removeNewFrameCallback(world);
            delete world;
        }
        delete stream;
    #else
        QMessageBox::critical(nullptr, "Unsupported", "You need to have Bullet Physics installed and the project configured as described in its readme in order to open the Augmented View.");
    #endif

    return returnValue;
}



// The graphical interface main
int mainInterface(QApplication& app)
{
    MainWindow* window = new MainWindow();
    window->show();

    int returnValue = app.exec();

    window->close();
    delete window;

    return returnValue;
}




//
// Application entry point
//
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    /*#if defined(_WIN32)
        AllocConsole(); // Open a console
    #endif*/

    
    MainFunction mainFunction = mainInterface;

    // Parse arguments
    std::map<std::string, std::string> args;
    std::vector<std::string> files;
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.size() > 2) {
            if (arg[0] == '-') {
                if (arg[1] == '-') { // Long option
                    arg = arg.substr(2);
                    if (arg == "augmented") mainFunction = mainAugmented;
                    //else if (arg == "videotest") mainFunction = videoTest;
                    else qDebug("Unknown option: \"--%s\"", arg.c_str());
                }
                else { // Short option(s)
                    for (size_t j = 1; j < arg.size(); ++j) {
                        switch (arg[j]) {
                            case 'a':
                                mainFunction = mainAugmented;
                                break;

                            default:
                                qDebug("Unknown option: \"-%c\"", arg[j]);
                        }
                    }
                }
            }
            else files.push_back(arg);
        }
    }

    // Run the main function
    int returnValue = mainFunction(app);

    #ifdef REPORT_MEMORY_LEAKS
        _CrtDumpMemoryLeaks();
    #endif

    return returnValue;
}

