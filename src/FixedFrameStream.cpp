#include "FixedFrameStream.h"

#include "Utils.h"

#include "SkeletonIO.h"

FixedFrameStream::FixedFrameStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile) :
    colorFrame(nullptr),
    depthFrame(nullptr),
    skeletonFrame()
{

    if (!colorFile.empty()) {
        cv::Mat img = cv::imread(colorFile);
        colorFrame = newColorFrame();
        Utils::rgbToColorFrame(img, colorFrame);

        if (name.empty()) name = colorFile;
    }

    if (!depthFile.empty()) {
        cv::Mat img = cv::imread(depthFile);
        depthFrame = newDepthFrame();
        Utils::rgbToDepthFrame(img, depthFrame);

        if (name.empty()) name = depthFile;
    }

    if (!skeletonFile.empty()) {
        // TODO
        SkeletonIO reader; 
		
		if (reader.openFileForReading(skeletonFile.c_str())) { 
			reader.readFrame(skeletonFrame); 
			if (name.empty()) name = skeletonFile; 
		}
    }

    if (name.empty()) name = "Unnamed";
    else {
        std::size_t pos = name.find_last_of("\\/");
        if (pos != std::string::npos) name = name.substr(pos + 1);
    }
}

FixedFrameStream::~FixedFrameStream()
{
    if (colorFrame != nullptr) delete[] colorFrame;
    if (depthFrame != nullptr) delete[] depthFrame;
}


bool FixedFrameStream::waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame, NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum)
{
    if (colorFrame != nullptr) memcpy(colorFrame, this->colorFrame, COLOR_FRAME_SIZE);
    if (depthFrame != nullptr) memcpy(depthFrame, this->depthFrame, DEPTH_FRAME_SIZE);
    if (skeletonFrame != nullptr) memcpy(skeletonFrame, &(this->skeletonFrame), sizeof(NUI_SKELETON_FRAME));

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

bool FixedFrameStream::getColorFrame(ColorPixel* data, FrameNum* frameNum)
{
    if (colorFrame == nullptr) return false;
    memcpy(data, colorFrame, COLOR_FRAME_SIZE);

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

bool FixedFrameStream::getColorImage(cv::Mat& mat, FrameNum* frameNum)
{
    if (colorFrame == nullptr) return false;
    Utils::colorFrameToRgb((ColorPixel*)colorFrame, mat);

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

void FixedFrameStream::setColorImage(cv::Mat& mat)
{
    if (colorFrame == nullptr) colorFrame = newColorFrame();
    Utils::rgbToColorFrame(mat, colorFrame);
}


bool FixedFrameStream::getDepthFrame(DepthPixel* data, FrameNum* frameNum)
{
    if (depthFrame == nullptr) return false;
    memcpy(data, depthFrame, DEPTH_FRAME_SIZE);

    if (frameNum == nullptr) return true;
    int v = frameNum->num;
    frameNum->num = 1;
    return v != 1;
}

bool FixedFrameStream::getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* frameNum)
{
    if (skeletonFrame.dwFrameNumber == 0) return false;

    int val = frame.dwFrameNumber;

    memcpy(&frame, &skeletonFrame, sizeof(NUI_SKELETON_FRAME));

    return val != frame.dwFrameNumber;
}
