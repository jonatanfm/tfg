#ifndef DEPTHCORRECTOR_H
#define DEPTHCORRECTOR_H

#pragma once

#include "../globals.h"

#include "../AsyncStream.h"

class DepthCorrectorStream : public AsyncStream
{
    private:
        Ptr<DataStream> base;

        static const int MIN_DEPTH = NUI_IMAGE_DEPTH_MINIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
        static const int MAX_DEPTH = NUI_IMAGE_DEPTH_MAXIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

    public:

        DepthCorrectorStream(Ptr<DataStream> baseStream) :
            base(baseStream)
        {
            depthFrame = newDepthFrame();

            start();
        }

        std::string getName() const override
        {
            return "Depth Noise Corrector";
        }

        bool isOpened() const override
        {
            return true;
        }

        void stream() override
        {
            DataStream::DepthPixel* input = newDepthFrame();
            DataStream::DepthPixel* output = newDepthFrame();
            while (!stopping)
            {
                base->waitForFrame(nullptr, input, nullptr);
    
                correctDepthFrame(input, output);

                pushFrame(nullptr, output, nullptr);
            }
            delete[] input;
            delete[] output;
        }

        static void correctDepthFrame(const DataStream::DepthPixel* source, DataStream::DepthPixel* target);

    private:

        static void correctDepthFrameA(const DataStream::DepthPixel* source, DataStream::DepthPixel* target);
        static void correctDepthFrameB(const DataStream::DepthPixel* source, DataStream::DepthPixel* target);
        static void correctDepthFrameC(const DataStream::DepthPixel* source, DataStream::DepthPixel* target);

};


#endif
