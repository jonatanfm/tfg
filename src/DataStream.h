#ifndef DATASTREAM_H
#define DATASTREAM_H

#pragma once

#include "globals.h"

#include "Calibration.h"

#include "Data.h"




// Base class for the different data streams
// As a plus, implements OpenCV compatibility with "VideoCapture" class
class DataStream : public cv::VideoCapture
{
    friend void Ptr<DataStream>::delete_obj();

    public:

        typedef std::function< void (const ColorFrame* color, const DepthFrame* depth, const SkeletonFrame* skeleton) > Callback;


        DataStream() :
            colorIntrinsics(),
            depthIntrinsics()
        {

        }

        virtual ~DataStream()
        {

        }

        // Returns true if the stream is active
        virtual bool isOpened() const override
        {
            return false;
        }

        // Returns the display name of the stream
        virtual std::string getName() const
        {
            return "Unnamed Stream"; // Default name
        }

        // Returns true if the stream provides color data
        virtual bool hasColor() const
        {
            return false;
        }

        // Returns true if the stream provides depth data
        virtual bool hasDepth() const
        {
            return false;
        }

        // Returns true if the stream provides skeleton data
        virtual bool hasSkeleton() const
        {
            return false;
        }

        virtual void addNewFrameCallback(void* owner, const Callback& callback)
        {
            newFrameCallbacks.push_back(std::make_pair(owner, callback));
        }

        virtual void removeNewFrameCallback(void* owner)
        {
            for (auto it = newFrameCallbacks.begin(); it != newFrameCallbacks.end();) {
                if (it->first == owner) it = newFrameCallbacks.erase(it);
                else ++it;
            }
        }


        // Puts the calling thread to sleep until a new frame is available
        // Once available, it is awakened and the non-null pointers passed
        // as arguments are filled with the new frame data.
        virtual bool waitForFrame(ColorFrame* colorFrame, DepthFrame* depthFrame,
            SkeletonFrame* skeletonFrame, FrameNum* frameNum = nullptr) = 0;

        // Get the most recent color frame
        virtual bool getColorFrame(ColorFrame& frame, FrameNum* num = nullptr)
        {
            return false;
        }

        // Get the most recent color frame, as an RGB image. Returns true if success.
        virtual bool getColorImage(cv::Mat& mat, FrameNum* num = nullptr)
        {
            return false;
        }

        // Get the most recent depth frame. Returns true if success.
        virtual bool getDepthFrame(DepthFrame& frame, FrameNum* num = nullptr)
        {
            return false;
        }

        //virtual bool getDepthImage(cv::Mat& mat, FrameNum* num = nullptr)
        //{
        //    return false;
        //}

        // Get the most recent skeleton frame. Returns true if success.
        virtual bool getSkeletonFrame(SkeletonFrame& frame, FrameNum* num = nullptr)
        {
            return false;
        }



        const IntrinsicParams& getColorIntrinsics() const
        {
            return colorIntrinsics;
        }

        const IntrinsicParams& getDepthIntrinsics() const
        {
            return depthIntrinsics;
        }

        void setColorIntrinsics(IntrinsicParams& intrinsics)
        {
            colorIntrinsics = intrinsics;
        }

        void setDepthIntrinsics(IntrinsicParams& intrinsics)
        {
            depthIntrinsics = intrinsics;
        }


        // Signals the object to be deleted
        // Returns true if "delete" should be called, or false if it will delete itself
        static void deleting(DataStream* stream); // Implemented in MainWindow.cpp, do not call manually



        virtual bool grab() override
        {
            return true;
        }

        virtual bool retrieve(CV_OUT cv::Mat& image, int channel = 0) override
        {
            if (image.type() != CV_8UC3 || image.size().width != ColorFrame::WIDTH || image.size().height != ColorFrame::HEIGHT) {
                image.create(cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT), CV_8UC3);
            }
            getColorImage(image);
            return true;
        }

        virtual bool read(CV_OUT cv::Mat& image) override
        {
            if (grab()) return retrieve(image, 0);
            return false;
        }

        DataStream& operator >> (CV_OUT cv::Mat& image) override
        {
            read(image);
            return *this;
        }

        // Unused
        void release() override
        {

        }

        // Unused
        virtual bool set(int propId, double value) override
        {
            return false;
        }

        // Unused
        virtual double get(int propId) override
        {
            return 0.0;
        }

        // Unused
        bool open(const cv::string& filename) override
        {
            return false;
        }

        // Unused
        bool open(int device) override
        {
            return false;
        }


    protected:

        std::vector< std::pair<void*, Callback> > newFrameCallbacks;

        IntrinsicParams colorIntrinsics;
        IntrinsicParams depthIntrinsics;

        inline void callNewFrameCallbacks(const ColorFrame* color, const DepthFrame* depth, const SkeletonFrame* skeleton)
        {
            for (auto it = newFrameCallbacks.begin(); it != newFrameCallbacks.end(); ++it) {
                it->second(color, depth, skeleton);
            }
        }

        // Called when "delete" has been called on this object
        // If returns true, the memory will be deallocated
        virtual void performDelete()
        {
            qDebug() << "Deleting Stream " << this;
            DataStream::deleting(this);
            delete this; // Commit sucicide
        }


};


// Override OpenCV's Ptr deleter for DataStreams
template<> inline void Ptr<DataStream>::delete_obj()
{
    if (obj != nullptr) obj->performDelete();
}


#endif
