#include "RecordedStream.h"

#include "Utils.h"

RecordedStream::RecordedStream(const std::string& color, const std::string& depth, const std::string& skeleton) :
    colorFile(color),
    depthFile(depth),
    skeletonFile(skeleton),
    resetting(false),
    paused(true),
    advancing(false)
{

    if (!colorFile.empty()) {
        colorVideo.open(colorFile);
        if (!colorVideo.isOpened()) colorFile.clear();
        else {
            recordedColorFrame.create(cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT), CV_8UC3);
            colorBuffer.reset(new ColorFrame());
            if (name.empty()) name = colorFile;
        }
    }

    if (!depthFile.empty()) {
        depthVideo.open(depthFile);
        if (!depthVideo.isOpened()) depthFile.clear();
        else {
            recordedDepthFrame.create(cv::Size(DepthFrame::WIDTH, DepthFrame::HEIGHT), CV_8UC3);
            depthBuffer.reset(new DepthFrame());
            if (name.empty()) name = depthFile;
        }
    }

    if (!skeletonFile.empty()) {
        if (!skeletonReader.openFileForReading(skeletonFile.c_str())) {
            skeletonFile.clear();
        }
        else {
            if (name.empty()) name = skeletonFile;
        }
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
    colorVideo.release();
    depthVideo.release();
    skeletonReader.close();
}

static const int FPS = 30;

void RecordedStream::stream()
{
    //qDebug("[RecordedStream] Thread started");

    ColorFrame* color;
    DepthFrame* depth;
    SkeletonFrame* skeleton;
    while (!stopping) {

        if (resetting) {
            resetting = false;
            //currentFrame = 0;
            colorVideo.open(colorFile);
            depthVideo.open(depthFile);
            skeletonReader.reset();
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

        depth = nullptr;
        if (depthVideo.isOpened() && depthVideo.grab() && depthVideo.retrieve(recordedDepthFrame)) {
            Utils::rgbToDepthFrame(recordedDepthFrame, *depthBuffer);
            depth = depthBuffer.get();
        }

        color = nullptr;
        if (colorVideo.isOpened() && colorVideo.grab() && colorVideo.retrieve(recordedColorFrame)) {
            Utils::rgbToColorFrame(recordedColorFrame, *colorBuffer);
            color = colorBuffer.get();
        }

        skeleton = nullptr;
        if (skeletonReader.isOpen() && skeletonReader.readFrame(recordedSkeletonFrame)) {
            skeleton = &recordedSkeletonFrame;
            
            if (!skeletonReader.isSmoothed() && skeletonSmoothing) NuiTransformSmooth(&skeleton->frame, nullptr);
        }

        pushFrame(color, depth, skeleton);
    }

    //qDebug("[RecordedStream] Thread exited");
}
