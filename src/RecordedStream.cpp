#include "RecordedStream.h"

#include "Utils.h"

RecordedStream::RecordedStream(const std::string& color, const std::string& depth, const std::string& skeleton) :
    colorFile(color),
    depthFile(depth),
    skeletonFile(skeleton),
    colorFrame(),
    depthFrame(),
    skeletonFrame(),
    currentFrame(0),
    running(false),
    resetting(false),
    paused(false),
    advancing(false)
{

    if (!colorFile.empty()) {
        colorVideo.open(colorFile);
        if (!colorVideo.isOpened()) colorFile.clear();
        else {
            colorFrame.create(cv::Size(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT), CV_8UC3);
            if (name.empty()) name = colorFile;
        }
    }

    if (!depthFile.empty()) {
        depthVideo.open(depthFile);
        if (!depthVideo.isOpened()) depthFile.clear();
        else {
            colorFrame.create(cv::Size(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT), CV_16UC1);
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

    this->start();
}

RecordedStream::~RecordedStream()
{
    release();
}

void RecordedStream::release()
{
    if (this->isRunning()) {
        running = false;
        this->wait();
    }
}


bool RecordedStream::waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame, NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum)
{
    bool signaled;
    newFrameMutex.lock();
    if (signaled = newFrameEvent.wait(&newFrameMutex)) {
        if (colorFrame != nullptr) Utils::rgbToColorFrame(this->colorFrame, colorFrame);
        if (depthFrame != nullptr) Utils::rgbToDepthFrame(this->depthFrame, depthFrame);
        //if (skeletonFrame != nullptr) memcpy(skeletonFrame, &skeleton, sizeof(NUI_SKELETON_FRAME));
    }
    newFrameMutex.unlock();
    return signaled;
}

bool RecordedStream::getColorFrame(ColorPixel* data, FrameNum* num)
{
    if (!colorVideo.isOpened()) return false;
    newFrameMutex.lock();
    Utils::rgbToColorFrame(depthFrame, data);
    newFrameMutex.unlock();
    return true;
}

bool RecordedStream::getColorImage(cv::Mat& mat, FrameNum* num)
{
    if (!colorVideo.isOpened()) return false;
    newFrameMutex.lock();
    colorFrame.copyTo(mat);
    newFrameMutex.unlock();
    return true;
}


bool RecordedStream::getDepthFrame(DepthPixel* data, FrameNum* num)
{
    if (!depthVideo.isOpened()) return false;
    newFrameMutex.lock();
    Utils::rgbToDepthFrame(depthFrame, data);
    newFrameMutex.unlock();
    return true;
}

bool RecordedStream::getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* num)
{
    // TODO - Stub
    return false;
}


static const int FPS = 30;

void RecordedStream::run()
{
    running = true;

    qDebug("[RecordedStream] Thread started");

    bool color, depth;
    while (true) {
        if (!running) break;

        if (resetting) {
            resetting = false;
            currentFrame = 0;
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

        color = colorVideo.isOpened() && colorVideo.grab();
        depth = depthVideo.isOpened() && depthVideo.grab();

        newFrameMutex.lock();

        if (color) colorVideo.retrieve(colorFrame);
        if (depth) depthVideo.retrieve(colorFrame);

        //updateSkeleton();

        ++currentFrame;

        newFrameEvent.wakeAll();
        newFrameMutex.unlock();
    }

    qDebug("[RecordedStream] Thread exited");
}
