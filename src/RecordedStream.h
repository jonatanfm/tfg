#ifndef RECORDEDSTREAM_H
#define RECORDEDSTREAM_H

#pragma once

#include "AsyncStream.h"

#include "SkeletonIO.h"

class RecordedStream : public AsyncStream
{
    public:
        RecordedStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile);

        ~RecordedStream();

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
            return !skeletonFile.empty();
        }

    private:

        std::string name;

        std::string colorFile;
        std::string depthFile;
        std::string skeletonFile;

        cv::VideoCapture colorVideo;
        cv::VideoCapture depthVideo;
        SkeletonIO skeletonReader;

        cv::Mat recordedColorFrame;
        cv::Mat recordedDepthFrame;
        SkeletonFrame recordedSkeletonFrame;

        std::unique_ptr<ColorFrame> colorBuffer;
        std::unique_ptr<DepthFrame> depthBuffer;
        
        volatile bool resetting;
        volatile bool paused;
        volatile bool advancing;

        void stream() override;

};

#endif

