#ifndef DATASTREAM_H
#define DATASTREAM_H

#pragma once

#include "globals.h"

// Base class for the different data streams
// As a plus, implements OpenCV compatibility with "VideoCapture" class
class DataStream : public cv::VideoCapture
{
    public:

        // Represents a frame number. Used to check if two frames are the same
        struct FrameNum
        {
            friend class FixedFrameStream;

            private:
                int num;

            public:
                FrameNum() : num(0) { }

                FrameNum(const FrameNum& other) : num(other.num) { }

                inline bool operator==(const FrameNum& other) const
                {
                    return num == other.num;
                }

                inline bool operator!=(const FrameNum& other) const
                {
                    return num != other.num;
                }

                inline void operator++()
                {
                    ++num;
                }
        };


        // A color frame pixel
        union ColorPixel
        {
            unsigned int color;
            unsigned char data[4];
            struct
            {
                unsigned char r;
                unsigned char g;
                unsigned char b;
                unsigned char a;
            };
        };

        // A depth frame pixel
        struct DepthPixel
        {
            unsigned short playerIndex;
            unsigned short depth;
        };

        // Size in bytes of a Color frame
        static const unsigned int COLOR_FRAME_SIZE = (COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT) * sizeof(ColorPixel);

        // Size in bytes of a Depth frame
        static const unsigned int DEPTH_FRAME_SIZE = (DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT) * sizeof(DepthPixel);

        DataStream()
        {

        }

        virtual ~DataStream()
        {
            release();
        }

        // Returns true if the stream is active
        virtual bool isOpened() const override
        {
            return false;
        }

        // Closes the stream and releases
        virtual void release() override
        {

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

        // Puts the calling thread to sleep until a new frame is available
        // Once available, it is awakened and the non-null pointers passed
        // as arguments are filled with the new frame data.
        virtual bool waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame,
            NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum = nullptr) = 0;

        // Get the most recent color frame
        virtual bool getColorFrame(ColorPixel* data, FrameNum* num = nullptr)
        {
            return false;
        }

        // Get the most recent color frame, as an RGB image. Returns true if success.
        virtual bool getColorImage(cv::Mat& mat, FrameNum* num = nullptr)
        {
            return false;
        }

        // Get the most recent depth frame. Returns true if success.
        virtual bool getDepthFrame(DepthPixel* data, FrameNum* num = nullptr)
        {
            return false;
        }

        //virtual bool getDepthImage(cv::Mat& mat, FrameNum* num = nullptr)
        //{
        //    return false;
        //}

        // Get the most recent skeleton frame. Returns true if success.
        virtual bool getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* num = nullptr)
        {
            return false;
        }


    public:

        static void deleting(DataStream* stream); // Implemented in MainWindow.cpp, do not call manually


        virtual bool grab() override
        {
            return true;
        }

        virtual bool retrieve(CV_OUT cv::Mat& image, int channel = 0) override
        {
            if (image.type() != CV_8UC3 || image.size().width != COLOR_FRAME_WIDTH || image.size().height != COLOR_FRAME_HEIGHT) {
                image.create(cv::Size(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT), CV_8UC3);
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

};


// Override OpenCV's Ptr deleter for DataStreams, to close them
// and remove them from MainWindow's "streams" vector, if it exists
template<> inline void Ptr<DataStream>::delete_obj()
{
    if (obj != nullptr)
    {
        DataStream::deleting(obj);
        delete obj;
    }
}


#endif
