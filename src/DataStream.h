#ifndef DATASTREAM_H
#define DATASTREAM_H

#pragma once

#include "globals.h"

#include "Calibration.h"

#include "Data.h"


// Base class for the different data streams.
// A stream provides color/depth/skeleton frames, which may be the result of processing another stream's output.
// As an extra, implements OpenCV compatibility with "VideoCapture" class
class DataStream : public cv::VideoCapture
{
    friend void Ptr<DataStream>::delete_obj(); // Defined at the bottom of the file

    public:

        // A callback function to register for new frame availability.
        // See C++11 Lambda Functions for an easy way to create these functions: [] (parameters) -> returnType { body; }
        typedef std::function< void (const ColorFrame* color, const DepthFrame* depth, const SkeletonFrame* skeleton) > Callback;


        DataStream() :
            colorIntrinsics(),
            depthIntrinsics(),
            skeletonSmoothing(true)
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

        // Returns true if the stream provides color frames
        virtual bool hasColor() const
        {
            return false;
        }

        // Returns true if the stream provides depth frames
        virtual bool hasDepth() const
        {
            return false;
        }

        // Returns true if the stream provides skeleton frames
        virtual bool hasSkeleton() const
        {
            return false;
        }


        // Registers a new callback function for when a new frame or set of frames is available.
        // WARNING: Keep in mind that these callbacks may be executed in another thread,
        // they are meant to be functions running a short code.
        // An example usage is emitting a QT signal to refresh a widget.
        // The "owner" parameter may be any pointer which can be used afterwards to remove the callbacks
        // (for example, the calling object "this" pointer).
        virtual void addNewFrameCallback(void* owner, const Callback& callback)
        {
            newFrameCallbacks.push_back(std::make_pair(owner, callback));
        }


        // Removes all registered callbacks functions added by the given owner. 
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

        // Get the most recent color frame. Returns true if success.
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


        // Gets the color camera intrinsic parameters.
        const IntrinsicParams& getColorIntrinsics() const
        {
            return colorIntrinsics;
        }

        // Gets the depth camera intrinsic parameters.
        const IntrinsicParams& getDepthIntrinsics() const
        {
            return depthIntrinsics;
        }

        // Sets the color camera intrinsic parameters.
        void setColorIntrinsics(IntrinsicParams& intrinsics)
        {
            colorIntrinsics = intrinsics;
        }

        // Sets the depth camera intrinsic parameters.
        void setDepthIntrinsics(IntrinsicParams& intrinsics)
        {
            depthIntrinsics = intrinsics;
        }

        // Get if skeleton smoothing is enabled
        bool getSkeletonSmoothing() const
        {
            return skeletonSmoothing;
        }

        // Enable/disable skeleton smoothing
        void setSkeletonSmoothing(bool smooth)
        {
            skeletonSmoothing = smooth;
        }



        // Signals the object to be deleted
        // Returns true if "delete" should be called, or false if it will delete itself
        static void deleting(DataStream* stream); // Implemented in MainWindow.cpp, do not call manually


        // Unused - Implemented for compatibility with cv::VideoCapture
        virtual bool grab() override
        {
            return true;
        }

        // Gets the most recent image frame - Implemented for compatibility with cv::VideoCapture
        virtual bool retrieve(CV_OUT cv::Mat& image, int channel = 0) override
        {
            if (image.type() != CV_8UC3 || image.size().width != ColorFrame::WIDTH || image.size().height != ColorFrame::HEIGHT) {
                image.create(cv::Size(ColorFrame::WIDTH, ColorFrame::HEIGHT), CV_8UC3);
            }
            getColorImage(image);
            return true;
        }

        // Gets the most recent image frame - Implemented for compatibility with cv::VideoCapture
        virtual bool read(CV_OUT cv::Mat& image) override
        {
            if (grab()) return retrieve(image, 0);
            return false;
        }

        // Gets the most recent image frame - Implemented for compatibility with cv::VideoCapture
        DataStream& operator >> (CV_OUT cv::Mat& image) override
        {
            read(image);
            return *this;
        }

        // Unused - Implemented for compatibility with cv::VideoCapture
        void release() override
        {

        }

        // Unused - Implemented for compatibility with cv::VideoCapture
        virtual bool set(int propId, double value) override
        {
            return false;
        }

        // Unused - Implemented for compatibility with cv::VideoCapture
        virtual double get(int propId) override
        {
            return 0.0;
        }

        // Unused - Implemented for compatibility with cv::VideoCapture
        bool open(const cv::string& filename) override
        {
            return false;
        }

        // Unused - Implemented for compatibility with cv::VideoCapture
        bool open(int device) override
        {
            return false;
        }


    protected:

        // List of registered new frame callback functions
        std::vector< std::pair<void*, Callback> > newFrameCallbacks;

        IntrinsicParams colorIntrinsics; // Color camera intrinsics
        IntrinsicParams depthIntrinsics; // Depth camera intrinsics

        volatile bool skeletonSmoothing; // Skeleton smoothing enabled?

        // Calls all registered new frame callback functions
        inline void callNewFrameCallbacks(const ColorFrame* color, const DepthFrame* depth, const SkeletonFrame* skeleton)
        {
            for (auto it = newFrameCallbacks.begin(); it != newFrameCallbacks.end(); ++it) {
                it->second(color, depth, skeleton);
            }
        }

        // Called when this object is to be deleted.
        // This allows the child classes which run in other threads to stop gracefully,
        // by deleting the actual object when the thread has also stopped.
        virtual void performDelete()
        {
            qDebug() << "Deleting Stream " << this;
            DataStream::deleting(this);
            delete this; // Commit suicide
        }


    private:

        DataStream(const DataStream&); // Disable copy constructor
        DataStream& operator=(const DataStream&); // Disable assignment

};


// Override OpenCV's Ptr deleter for DataStreams
template<> inline void Ptr<DataStream>::delete_obj()
{
    if (obj != nullptr) obj->performDelete();
}


#endif
