#ifndef RECORDEDSTREAM_H
#define RECORDEDSTREAM_H

#pragma once

#include "DataStream.h"

class RecordedStream : public DataStream, private QThread
{
    public:
        RecordedStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile);
        ~RecordedStream();

        void release() override;

        bool waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame, NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum = nullptr) override;

        bool getColorFrame(ColorPixel* data, FrameNum* num = nullptr) override;
        bool getColorImage(cv::Mat& mat, FrameNum* num = nullptr) override;

        bool getDepthFrame(DepthPixel* data, FrameNum* num = nullptr) override;

        bool getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* num = nullptr) override;



        void reset()
        {
            resetting = true;
        }

        void setPaused(bool paused)
        {
            paused = true;
        }

        void advance()
        {
            advancing = true;
        }



        std::string getName() const override
        {
            return name;
        }

        bool isOpened() const override
        {
            return true;
        }

        bool hasColor() const override
        {
            return !colorFile.empty();
        }

        bool hasDepth() const override
        {
            return !depthFile.empty();
        }

        bool hasSkeleton() const override
        {
            return skeletonFrame.dwFrameNumber != 0;
        }

    private:

        std::string name;

        std::string colorFile;
        std::string depthFile;
        std::string skeletonFile;

        cv::VideoCapture colorVideo;
        cv::VideoCapture depthVideo;

        cv::Mat colorFrame;
        cv::Mat depthFrame;


        NUI_SKELETON_FRAME skeletonFrame;

        QWaitCondition newFrameEvent;
        QMutex newFrameMutex;

        int currentFrame;

        volatile bool running;
        volatile bool resetting;
        volatile bool paused;
        volatile bool advancing;

        void run() override;

};

#endif

