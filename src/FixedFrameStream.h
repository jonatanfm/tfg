#ifndef FIXEDFRAMESTREAM_H
#define FIXEDFRAMESTREAM_H

#pragma once

#include "DataStream.h"

class FixedFrameStream : public DataStream
{
    public:
        FixedFrameStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile);


        bool waitForFrame(ColorFrame* colorFrame, DepthFrame* depthFrame, SkeletonFrame* skeletonFrame, FrameNum* frameNum = nullptr) override;

        bool getColorFrame(ColorFrame& frame, FrameNum* num = nullptr) override;
        bool getColorImage(cv::Mat& mat, FrameNum* num = nullptr) override;

        bool getDepthFrame(DepthFrame& frame, FrameNum* num = nullptr) override;

        bool getSkeletonFrame(SkeletonFrame& frame, FrameNum* num = nullptr) override;


        void setColorImage(cv::Mat& mat);

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

        std::string name;

        std::unique_ptr<ColorFrame> colorFrame;
        std::unique_ptr<DepthFrame> depthFrame;
        std::unique_ptr<SkeletonFrame> skeletonFrame;
};

#endif

