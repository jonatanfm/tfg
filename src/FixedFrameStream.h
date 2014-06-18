#ifndef FIXEDFRAMESTREAM_H
#define FIXEDFRAMESTREAM_H

#pragma once

#include "DataStream.h"

// Stream that holds a single fixed frame.
class FixedFrameStream : public DataStream
{
    public:
        // Create a new FixedFrameStream opening the given files. If an empty string is passed it is ignored.
        FixedFrameStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile);


        bool waitForFrame(ColorFrame* colorFrame, DepthFrame* depthFrame, SkeletonFrame* skeletonFrame, FrameNum* frameNum = nullptr) override;

        bool getColorFrame(ColorFrame& frame, FrameNum* num = nullptr) override;
        bool getColorImage(cv::Mat& mat, FrameNum* num = nullptr) override;

        bool getDepthFrame(DepthFrame& frame, FrameNum* num = nullptr) override;

        bool getSkeletonFrame(SkeletonFrame& frame, FrameNum* num = nullptr) override;


        // Set the color frame. Used mainly for debugging.
        void setColorImage(cv::Mat& mat);


        // Trigger manually a frame update, to refresh other views
        void relaunchFrame()
        {
            callNewFrameCallbacks(colorFrame.get(), depthFrame.get(), skeletonFrame.get());
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
            return colorFrame != nullptr;
        }

        bool hasDepth() const override
        {
            return depthFrame != nullptr;
        }

        bool hasSkeleton() const override
        {
            return skeletonFrame != nullptr;
        }

    private:

        std::string name; // Stream name, derived from one of the open files

        std::unique_ptr<ColorFrame> colorFrame; // The held color frame, or null
        std::unique_ptr<DepthFrame> depthFrame; // The held depth frame, or null
        std::unique_ptr<SkeletonFrame> skeletonFrame; // The held skeleton frame, or null
};

#endif

