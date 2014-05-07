#ifndef ASYNCSTREAM_H
#define ASYNCSTREAM_H

#pragma once

#include "globals.h"

#include <QThread>
#include <QMutex>

#include "Utils.h"

#include "DataStream.h"

class AsyncStream : public DataStream, private QThread
{
    public:

        AsyncStream() :
            colorFrame(nullptr),
            depthFrame(nullptr),
            skeletonFrame(nullptr),
            currentFrame(),
            stopping(false)
        {

        }

        virtual ~AsyncStream()
        {
            release();
            if (colorFrame != nullptr) delete[] colorFrame;
            if (depthFrame != nullptr) delete[] depthFrame;
            if (skeletonFrame != nullptr) delete skeletonFrame;
        }

        void release() override
        {
            if (this->isRunning()) {
                stopping = true;
                this->wait();
            }
        }


        void start()
        {
            QThread::start();
        }


        virtual bool hasColor() const override
        {
            return colorFrame != nullptr;
        }

        virtual bool hasDepth() const override
        {
            return depthFrame != nullptr;
        }

        virtual bool hasSkeleton() const override
        {
            return skeletonFrame != nullptr;
        }


        bool waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame, NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum) override
        {
            mutex.lock();
            bool newFrame = nextFrame.wait(&mutex);
            if (newFrame) {
                if (colorFrame != nullptr && this->colorFrame != nullptr) {
                    memcpy(colorFrame, this->colorFrame, COLOR_FRAME_SIZE);
                }
                if (depthFrame != nullptr && this->depthFrame != nullptr) {
                    memcpy(depthFrame, this->depthFrame, DEPTH_FRAME_SIZE);
                }
                if (skeletonFrame != nullptr && this->skeletonFrame != nullptr) {
                    memcpy(skeletonFrame, this->skeletonFrame, sizeof(NUI_SKELETON_FRAME));
                }
            }
            mutex.unlock();
            return newFrame;
        }


        bool getColorFrame(ColorPixel* data, FrameNum* num) override
        {
            if (colorFrame == nullptr) return false;
            bool advanced = false;
            mutex.lock();
            if (num == nullptr || *num != currentFrame) {
                advanced = true;
                if (num != nullptr) *num = currentFrame;
                memcpy(data, colorFrame, COLOR_FRAME_SIZE);
            }
            mutex.unlock();
            return true;
        }

        bool getColorImage(cv::Mat& mat, FrameNum* num) override
        {
            if (colorFrame == nullptr) return false;
            bool advanced = false;
            mutex.lock();
            if (num == nullptr || *num != currentFrame) {
                advanced = true;
                if (num != nullptr) *num = currentFrame;
                Utils::colorFrameToRgb(colorFrame, mat);
            }
            mutex.unlock();
            return advanced;
        }

        bool getDepthFrame(DepthPixel* data, FrameNum* num) override
        {
            if (depthFrame == nullptr) return false;
            bool advanced = false;
            mutex.lock();
            if (num == nullptr || *num != currentFrame) {
                advanced = true;
                if (num != nullptr) *num = currentFrame;
                memcpy(data, depthFrame, DEPTH_FRAME_SIZE);
            }
            mutex.unlock();
            return advanced;
        }

        bool getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* num) override
        {
            if (skeletonFrame == nullptr) return false;
            bool advanced = false;
            mutex.lock();
            if (num == nullptr || *num != currentFrame) {
                advanced = true;
                if (num != nullptr) *num = currentFrame;
                memcpy(&frame, skeletonFrame, sizeof(NUI_SKELETON_FRAME));
            }
            mutex.unlock();
            return advanced;
        }


    private:

        QWaitCondition nextFrame;
        QMutex mutex;

    protected:

        FrameNum currentFrame;

        ColorPixel* colorFrame;
        DepthPixel* depthFrame;
        NUI_SKELETON_FRAME* skeletonFrame;

        volatile bool stopping;


        virtual void run() = 0;


        void beginFrame()
        {
            mutex.lock();
        }

        void endFrame()
        {
            nextFrame.wakeAll();
            mutex.unlock();
        }

        void pushFrame(const ColorPixel* color, const DepthPixel* depth, const NUI_SKELETON_FRAME* skeleton)
        {
            mutex.lock();

            if (color != nullptr) {
                if (colorFrame == nullptr) colorFrame = new ColorPixel[COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT];
                memcpy(colorFrame, color, COLOR_FRAME_SIZE);
            }
            if (depth != nullptr) {
                if (depthFrame == nullptr) depthFrame = new DepthPixel[DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT];
                memcpy(depthFrame, depth, DEPTH_FRAME_SIZE);
            }
            if (skeleton != nullptr) {
                if (skeletonFrame == nullptr) skeletonFrame = new NUI_SKELETON_FRAME();
                memcpy(skeletonFrame, skeleton, sizeof(NUI_SKELETON_FRAME));
            }
            ++currentFrame;

            nextFrame.wakeAll();
            mutex.unlock();
        }


};


#endif
