#ifndef KINECTSTREAM_H
#define KINECTSTREAM_H

#pragma once

#include "AsyncStream.h"

#include "globals.h"
#include "RenderUtils.h"

#pragma comment(lib, "kinect10")
#include <NuiApi.h>
#include <NuiSensor.h>
#include <NuiSkeleton.h>
#include <NuiImageCamera.h>

class KinectStream : public AsyncStream
{
    //friend void runUpdaterWrapper(KinectStream* self);
    friend class KinectManager;

    public:

        KinectStream();
        ~KinectStream();

        bool initialize(int sensorIndex = 0);
        bool initializeById(const OLECHAR* id);

        bool isOpened() const override
        {
            return sensor != nullptr;
        }

        bool hasColor() const override
        {
            return true; // colorStream != nullptr;
        }

        bool hasDepth() const override
        {
            return true; // depthStream != nullptr;
        }

        bool hasSkeleton() const override
        {
            return true;
        }

        std::string getName() const override
        {
            return "Kinect" + sensor->NuiInstanceIndex();
        }

    private:
        INuiSensor* sensor;

        HANDLE hEvent;

        HANDLE colorStream;
        HANDLE depthStream;

        ColorPixel colorBuffer[COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT];
        DepthPixel depthBuffer[DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT];
        NUI_SKELETON_FRAME skeletonBuffer;

        KinectStream(const KinectStream&);
        KinectStream& operator=(const KinectStream&);

        bool initializeStreams();

        void updateColorBuffer();
        void updateDepthBuffer();
        void updateSkeleton();

        void stream() override;

        void stop() override;
};

#endif
