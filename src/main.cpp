
#include <QApplication>

#include "MainWindow.h"

/*bool videoTest()
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


// Program entry point
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    /*#if defined(_WIN32)
        AllocConsole(); // Open a console
    #endif*/

    //videoTest();

    int returnValue;
    {
        MainWindow* window = new MainWindow();
        window->show();

        returnValue = app.exec();

        window->close();
        delete window;
    }

    return returnValue;
}

