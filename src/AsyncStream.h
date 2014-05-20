#ifndef ASYNCSTREAM_H
#define ASYNCSTREAM_H

#pragma once

#include "globals.h"

#include <QThread>
#include <QMutex>

#include "Utils.h"

#include "DataStream.h"

// Stream which asynchronously runs a thread to read, process and write frames
class AsyncStream : private QThread, public DataStream
{
    Q_OBJECT

    public:

        AsyncStream() :
            colorFrame(),
            depthFrame(),
            skeletonFrame(),
            currentFrame(),
            stopping(false),
            refs(1)
        {
            setObjectName("AsyncStream");
        }

        virtual ~AsyncStream()
        {
            if (colorFrame != nullptr) delete colorFrame;
            if (depthFrame != nullptr) delete depthFrame;
            if (skeletonFrame != nullptr) delete skeletonFrame;
        }


        void start()
        {
            if (!this->isRunning()) {
                connect(this, SIGNAL(finished()), this, SLOT(threadFinished()));
                QThread::start();
            }
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

        void addNewFrameCallback(void* owner, const Callback& callback) override
        {
            mutex.lock();
            DataStream::addNewFrameCallback(owner, callback);
            mutex.unlock();
        }

        void removeNewFrameCallback(void* owner) override
        {
            mutex.lock();
            DataStream::removeNewFrameCallback(owner);
            mutex.unlock();
        }


        bool waitForFrame(ColorFrame* colorFrame, DepthFrame* depthFrame, SkeletonFrame* skeletonFrame, FrameNum* frameNum) override
        {
            mutex.lock();
            bool newFrame = nextFrame.wait(&mutex);
            if (newFrame) {
                if (colorFrame != nullptr && this->colorFrame != nullptr) {
                    *colorFrame = *this->colorFrame;
                }
                if (depthFrame != nullptr && this->depthFrame != nullptr) {
                    *depthFrame = *this->depthFrame;
                }
                if (skeletonFrame != nullptr && this->skeletonFrame != nullptr) {
                    *skeletonFrame = *this->skeletonFrame;
                }
            }
            mutex.unlock();
            return newFrame;
        }


        bool getColorFrame(ColorFrame& frame, FrameNum* num) override
        {
            if (colorFrame == nullptr) return false;
            bool advanced = false;
            mutex.lock();
            if (num == nullptr || *num != currentFrame) {
                advanced = true;
                if (num != nullptr) *num = currentFrame;
                frame = *colorFrame;
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
                Utils::colorFrameToRgb(*colorFrame, mat);
            }
            mutex.unlock();
            return advanced;
        }

        bool getDepthFrame(DepthFrame& frame, FrameNum* num) override
        {
            if (depthFrame == nullptr) return false;
            bool advanced = false;
            mutex.lock();
            if (num == nullptr || *num != currentFrame) {
                advanced = true;
                if (num != nullptr) *num = currentFrame;
                frame = *depthFrame;
            }
            mutex.unlock();
            return advanced;
        }

        bool getSkeletonFrame(SkeletonFrame& frame, FrameNum* num) override
        {
            if (skeletonFrame == nullptr) return false;
            bool advanced = false;
            mutex.lock();
            if (num == nullptr || *num != currentFrame) {
                advanced = true;
                if (num != nullptr) *num = currentFrame;
                frame = *skeletonFrame;
            }
            mutex.unlock();
            return advanced;
        }


    private:

        QWaitCondition nextFrame;
        QMutex mutex;

    protected:

        FrameNum currentFrame;

        ColorFrame* colorFrame;
        DepthFrame* depthFrame;
        SkeletonFrame* skeletonFrame;

        int refs;
        volatile bool stopping;


        void run() override
        {
            ++refs;
            stream();
        }

        virtual void stream() = 0;


        // Called to stop the thread
        virtual void stop()
        {
            stopping = true;
        }

        void beginFrame()
        {
            mutex.lock();
        }

        void endFrame()
        {
            nextFrame.wakeAll();
            callNewFrameCallbacks(colorFrame, depthFrame, skeletonFrame);
            mutex.unlock();
        }

        void pushFrame(const ColorFrame* color, const DepthFrame* depth, const SkeletonFrame* skeleton)
        {
            mutex.lock();

            if (color != nullptr) {
                if (colorFrame == nullptr) colorFrame = new ColorFrame();
                *colorFrame = *color;
            }
            if (depth != nullptr) {
                if (depthFrame == nullptr) depthFrame = new DepthFrame();
                *depthFrame = *depth;
            }
            if (skeleton != nullptr) {
                if (skeletonFrame == nullptr) skeletonFrame = new SkeletonFrame();
                *skeletonFrame = *skeleton;
            }
            ++currentFrame;

            nextFrame.wakeAll();
            callNewFrameCallbacks(colorFrame, depthFrame, skeletonFrame);
            mutex.unlock();
        }

        virtual void performDelete() override
        {
            qDebug() << "Attempting delete of " << this;
            stop();
            if (CV_XADD(&refs, -1) <= 1) DataStream::performDelete();
        }

    private slots:

        void threadFinished()
        {
            performDelete();
        }
};


#endif
