#ifndef FIXEDFRAMESTREAM_H
#define FIXEDFRAMESTREAM_H

#pragma once

#include "DataStream.h"

class FixedFrameStream : public DataStream
{
    public:
        FixedFrameStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile);
        ~FixedFrameStream();


        bool waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame, NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum = nullptr) override;

        bool getColorFrame(ColorPixel* data, FrameNum* num = nullptr) override;
        bool getColorImage(cv::Mat& mat, FrameNum* num = nullptr) override;

        bool getDepthFrame(DepthPixel* data, FrameNum* num = nullptr) override;

        bool getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* num = nullptr) override;


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
            return skeletonFrame.dwFrameNumber != 0;
        }

    private:

        std::string name;

        ColorPixel* colorFrame;

        DepthPixel* depthFrame;

        NUI_SKELETON_FRAME skeletonFrame;
};

#endif

