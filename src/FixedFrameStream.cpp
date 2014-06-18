#include "FixedFrameStream.h"

#include "Utils.h"

#include "SkeletonIO.h"

FixedFrameStream::FixedFrameStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile) :
    colorFrame(),
    depthFrame(),
    skeletonFrame()
{

    if (!colorFile.empty()) {
        cv::Mat img = cv::imread(colorFile);
        colorFrame.reset(new ColorFrame());
        Utils::rgbToColorFrame(img, *colorFrame);

        if (name.empty()) name = colorFile;
    }

    if (!depthFile.empty()) {
        cv::Mat img = cv::imread(depthFile);
        depthFrame.reset(new DepthFrame());
        Utils::rgbToDepthFrame(img, *depthFrame);

        if (name.empty()) name = depthFile;
    }

    if (!skeletonFile.empty()) {
        SkeletonIO reader;
        SkeletonFrame* skf = new SkeletonFrame();
        if (reader.openFileForReading(skeletonFile.c_str()) && reader.readFrame(*skf)) {
            skeletonFrame.reset(skf);
            if (name.empty()) name = skeletonFile;
        }
        else delete skf;
    }

    if (name.empty()) name = "Unnamed";
    else {
        std::size_t pos = name.find_last_of("\\/");
        if (pos != std::string::npos) name = name.substr(pos + 1);
    }
}


bool FixedFrameStream::waitForFrame(ColorFrame* colorFrame, DepthFrame* depthFrame, SkeletonFrame* skeletonFrame, FrameNum* frameNum)
{
    QThread::msleep(10);
    if (colorFrame != nullptr) *colorFrame = *this->colorFrame;
    if (depthFrame != nullptr) *depthFrame = *this->depthFrame;
    if (skeletonFrame != nullptr) *skeletonFrame = *this->skeletonFrame;

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

bool FixedFrameStream::getColorFrame(ColorFrame& frame, FrameNum* frameNum)
{
    if (colorFrame == nullptr) return false;
    frame = *colorFrame;

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

bool FixedFrameStream::getColorImage(cv::Mat& mat, FrameNum* frameNum)
{
    if (colorFrame == nullptr) return false;
    Utils::colorFrameToRgb(*colorFrame, mat);

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

void FixedFrameStream::setColorImage(cv::Mat& mat)
{
    if (colorFrame == nullptr) colorFrame.reset(new ColorFrame());
    Utils::rgbToColorFrame(mat, *colorFrame);
}


bool FixedFrameStream::getDepthFrame(DepthFrame& frame, FrameNum* frameNum)
{
    if (depthFrame == nullptr) return false;
    frame = *depthFrame;

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

bool FixedFrameStream::getSkeletonFrame(SkeletonFrame& frame, FrameNum* frameNum)
{
    if (skeletonFrame == nullptr) return false;

    int val = frame.frame.dwFrameNumber;
    frame = *skeletonFrame;
    return val != frame.frame.dwFrameNumber;
}
