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
            setObjectName("AsyncStream"); // Set the thread name
        }

        virtual ~AsyncStream()
        {
            if (colorFrame != nullptr) delete colorFrame;
            if (depthFrame != nullptr) delete depthFrame;
            if (skeletonFrame != nullptr) delete skeletonFrame;
        }


        // Start the underlying thread (i.e. start streaming)
        void start()
        {
            if (!this->isRunning()) {
                connect(this, SIGNAL(finished()), this, SLOT(threadFinished()));
                QThread::start();
            }
        }



        // This stream has color/depth/skeleton frames if one has been created.
        // The child class constructors should create them in order for the
        // color/depth windows to be open automatically.

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



        // The following functions override their counterparts in DataStream
        // in order to use a mutexes and avoid threading problems.


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

        // Mutex used to lock cross-thread accesses
        QMutex mutex;

        // Allows notification of new frames availbility to subscribed threads
        QWaitCondition nextFrame;
        

        // Overrides QThread run()
        void run() override
        {
            ++refs;
            stream();
        }


    protected:

        FrameNum currentFrame;

        
        // Buffered color frame provided to other streams and updated with "pushFrame(...)". Don't change manually unless you know what you are doing.
        ColorFrame* colorFrame;

        // Buffered depth frame provided to other streams and updated with "pushFrame(...)". Don't change manually unless you know what you are doing.
        DepthFrame* depthFrame;

        // Buffered skeleton frame provided to other streams and updated with "pushFrame(...)". Don't change manually unless you know what you are doing.
        SkeletonFrame* skeletonFrame;

        // Number of references to this item updated with "performDelete()". Don't change manually unless you know what you are doing.
        int refs;

        // Boolean condition telling the thread that it should stop.
        // Normally should be used as looping condition for the "main" while inside the overriden "stream()" function.
        volatile bool stopping;


        // Function to read/process/write frames implemented by child classes.
        // Use streams passed through the constructor to read frames and emit the results
        // as new frames using "pushFrame(...)" (or manually).
        virtual void stream() = 0;


        // Called to stop the thread
        virtual void stop()
        {
            stopping = true;
        }

        // Begin pushing manually new frame(s) to other streams.
        void beginFrame()
        {
            mutex.lock();
        }

        // End pushing manually new frame(s) to other streams.
        void endFrame()
        {
            nextFrame.wakeAll();
            callNewFrameCallbacks(colorFrame, depthFrame, skeletonFrame);
            mutex.unlock();
        }

        // Push new processed frame(s) to other streams.
        // If a paramater is NULL (nullptr), it is ignored; otherwise the frame is pushed.
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


        // Reimplements DataStream's counterpart updating the reference count.
        // One reference is given to external pointers to this object,
        // and the other one is given to the thread running on this object (if any).
        // When no references are left, DataStream::performDelete() is called, which deletes the object.
        virtual void performDelete() override
        {
            //qDebug() << "Attempting delete of " << this;
            stop();
            if (CV_XADD(&refs, -1) <= 1) DataStream::performDelete();
        }

    private slots:

        // Called when the thread has exited.
        void threadFinished()
        {
            performDelete();
        }

};


#endif
