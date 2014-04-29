#ifndef KINECTSTREAM_H
#define KINECTSTREAM_H

#pragma once

#include "DataStream.h"

#include <QThread>
#include <QMutex>

#include "globals.h"
#include "RenderUtils.h"

#pragma comment(lib, "kinect10")
#include <NuiApi.h>
#include <NuiSensor.h>
#include <NuiSkeleton.h>
#include <NuiImageCamera.h>

class KinectStream : public DataStream, private QThread
{
    //friend void runUpdaterWrapper(KinectStream* self);
    friend class KinectManager;

    public:

        KinectStream();
        ~KinectStream();

        bool initialize(int sensorIndex = 0);
        bool initializeById(const OLECHAR* id);

        void release() override;

        bool isOpened() const override
        {
            return sensor != nullptr;
        }

        bool hasColor() const override
        {
            return colorStream != nullptr;
        }

        bool hasDepth() const override
        {
            return depthStream != nullptr;
        }

        bool hasSkeleton() const override
        {
            return true;
        }

        std::string getName() const override
        {
            return "Kinect" + sensor->NuiInstanceIndex();
        }


        bool waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame, NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum = nullptr) override;

        bool getColorFrame(ColorPixel* data, FrameNum* num = nullptr) override;
        bool getColorImage(cv::Mat& mat, FrameNum* num = nullptr) override;

        bool getDepthFrame(DepthPixel* data, FrameNum* num = nullptr) override;
        //bool getDepthImage(cv::Mat& mat, FrameNum* num = nullptr) override;

        bool getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* num = nullptr) override;

    private:
        INuiSensor* sensor;

        HANDLE hEvent;

        HANDLE colorStream;
        HANDLE depthStream;

        FrameNum colorFrameNum;
        FrameNum depthFrameNum;
        FrameNum skeletonFrameNum;

        unsigned char colorBuffer[COLOR_FRAME_SIZE];
        unsigned char depthBuffer[DEPTH_FRAME_SIZE];

        NUI_SKELETON_FRAME skeleton;

        volatile bool updating;

        QWaitCondition newFrameEvent;
        QMutex newFrameMutex, colorMutex, depthMutex, skeletonMutex;

        KinectStream(const KinectStream&);
        KinectStream& operator=(const KinectStream&);

        bool initializeStreams();

        void updateColorBuffer();
        void updateDepthBuffer();
        void updateSkeleton();

        void run() override;
};

#endif
