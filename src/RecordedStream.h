#ifndef RECORDEDSTREAM_H
#define RECORDEDSTREAM_H

#pragma once

#include "AsyncStream.h"

#include "SkeletonIO.h"

// Stream that replays a previously recorded video or set of frames.
class RecordedStream : public AsyncStream
{
    public:
        // Create a new RecordedStream opening the given files. If an empty string is passed it is ignored.
        RecordedStream(const std::string& colorFile, const std::string& depthFile, const std::string& skeletonFile);

        ~RecordedStream();

        // Restarts the recording from the beginning.
        void reset()
        {
            resetting = true;
        }

        // Sets whether this stream is paused or running.
        void setPaused(bool paused)
        {
            paused = true;
        }

        // Sets this stream to advance frame automatically.
        void advance()
        {
            advancing = true;
        }



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
            return !colorFile.empty();
        }

        bool hasDepth() const override
        {
            return !depthFile.empty();
        }

        bool hasSkeleton() const override
        {
            return !skeletonFile.empty();
        }

    private:

        std::string name; // The stream name derived from the open files.

        std::string colorFile; // The color video file.
        std::string depthFile; // The depth video file.
        std::string skeletonFile; // The skeleton frame set file.

        cv::VideoCapture colorVideo; // The color video reader.
        cv::VideoCapture depthVideo; // The depth video reader.
        SkeletonIO skeletonReader; // The skeleton frame set reader.

        cv::Mat recordedColorFrame; // Temporary buffer for read color frames.
        cv::Mat recordedDepthFrame; // Temporary buffer for read depth frames.
        SkeletonFrame recordedSkeletonFrame; // Temporary buffer for read skeleton frames.

        std::unique_ptr<ColorFrame> colorBuffer; // Temporary buffer for color frames.
        std::unique_ptr<DepthFrame> depthBuffer; // Temporary buffer for depth frames.
        
        volatile bool resetting; // Control variable to reset the stream.
        volatile bool paused; // Control variable to pause the stream.
        volatile bool advancing; // Control variable to automatically advance the stream.

        void stream() override;

};

#endif

