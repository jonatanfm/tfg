#include "RecordedStream.h"

#include "Utils.h"

RecordedStream::RecordedStream(const std::string& color, const std::string& depth, const std::string& skeleton) :
    colorFile(color),
    depthFile(depth),
    skeletonFile(skeleton),
    resetting(false),
    paused(false),
    advancing(false)
{

    if (!colorFile.empty()) {
        colorVideo.open(colorFile);
        if (!colorVideo.isOpened()) colorFile.clear();
        else {
            colorFrame.create(cv::Size(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT), CV_8UC3);
            colorBuffer = newColorFrame();
            if (name.empty()) name = colorFile;
        }
    }

    if (!depthFile.empty()) {
        depthVideo.open(depthFile);
        if (!depthVideo.isOpened()) depthFile.clear();
        else {
            depthFrame.create(cv::Size(DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT), CV_16UC1);
            depthBuffer = newDepthFrame();
            if (name.empty()) name = depthFile;
        }
    }

    if (!skeletonFile.empty()) {
        // TODO
        if (name.empty()) name = skeletonFile;
    }

    if (name.empty()) name = "Unnamed";
    else {
        std::size_t pos = name.find_last_of("\\/");
        if (pos != std::string::npos) name = name.substr(pos + 1);
    }

    start();
}

RecordedStream::~RecordedStream()
{
    if (colorBuffer != nullptr) delete[] colorBuffer;
    if (depthBuffer != nullptr) delete[] depthBuffer;
}



static const int FPS = 30;

void RecordedStream::run()
{
    qDebug("[RecordedStream] Thread started");

    ColorPixel* color;
    DepthPixel* depth;
    while (true) {
        if (stopping) break;

        if (resetting) {
            resetting = false;
            //currentFrame = 0;
            colorVideo.open(colorFile);
            depthVideo.open(depthFile);
            // todo
        }

        if (!paused) Sleep(1000 / FPS);
        else {
            while (paused) {
                if (advancing) {
                    advancing = false;
                    break;
                }
                Sleep(100);
            }
        }

        color = nullptr;
        if (colorVideo.isOpened() && colorVideo.grab()) {
            colorVideo.retrieve(colorFrame);
            Utils::rgbToColorFrame(colorFrame, colorBuffer);
            color = colorBuffer;
        }

        depth = nullptr;
        if (depthVideo.isOpened() && depthVideo.grab()) {
            depthVideo.retrieve(depthFrame);
            Utils::rgbToDepthFrame(depthFrame, depthBuffer);
            depth = depthBuffer;
        }

        //updateSkeleton(); // TODO

        pushFrame(color, depth, nullptr);
    }

    qDebug("[RecordedStream] Thread exited");
}
