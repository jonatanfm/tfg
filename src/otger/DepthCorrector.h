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
            depthFrame = new DepthFrame();

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
            DepthFrame* input = new DepthFrame();
            DepthFrame* output = new DepthFrame();
            while (!stopping)
            {
                base->waitForFrame(nullptr, input, nullptr);
    
                correctDepthFrame(*input, *output);

                pushFrame(nullptr, output, nullptr);
            }
            delete input;
            delete output;
        }

        static void correctDepthFrame(const DepthFrame& source, DepthFrame& target);

    private:

        static void correctDepthFrameA(const DepthFrame& source, DepthFrame& target);
        static void correctDepthFrameB(const DepthFrame& source, DepthFrame& target);
        static void correctDepthFrameC(const DepthFrame& source, DepthFrame& target);

};


#endif
