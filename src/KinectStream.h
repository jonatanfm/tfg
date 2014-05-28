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

// Stream that reads and provides data from a single Kinect device
class KinectStream : public AsyncStream
{
    //friend void runUpdaterWrapper(KinectStream* self);
    friend class KinectManager;

    public:

        KinectStream();
        ~KinectStream();

        // Initialize to the Kinect with the given index.
        bool initialize(int sensorIndex = 0);

        // Initialize to the Kinect with the given connection ID.
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
            char buffer[32];
            sprintf(buffer, "Kinect %d", sensor->NuiInstanceIndex());
            return buffer;
        }

        // Returns, for all points in a color frame, the corresponding depth values.
        bool mapColorFrameToDepthFrame(const DepthFrame& frame, OUT NUI_DEPTH_IMAGE_POINT* mapped);

    private:
        INuiSensor* sensor; // The internal sensor object

        HANDLE hEvent; // Event signaled when a new frame is available

        HANDLE colorStream; // Handle to the internal color stream
        HANDLE depthStream; // Handle to the internal depth stream

        INuiCoordinateMapper* mapper; // The device coordinates mapper (for mapping color/depth/skeleton frames)

        // Input frame buffers

        ColorFrame colorBuffer;
        DepthFrame depthBuffer;
        SkeletonFrame skeletonBuffer;


        // Initialize the internal Kinect streams.
        bool initializeStreams();


        void updateColorBuffer(); // Read a single color frame from the device.
        void updateDepthBuffer(); // Read a single depth frame from the device.
        void updateSkeleton(); // Read a single skeleton frame from the device.

        void stream() override;

        void stop() override;
};

#endif
