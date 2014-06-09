#ifndef DEPTHCORRECTOR_H
#define DEPTHCORRECTOR_H

#pragma once

#include "../globals.h"

#include "../AsyncStream.h"


class DepthCorrector
{
    private:
        DepthCorrector();

        static const int MIN_DEPTH = DepthFrame::MIN_DEPTH;
        static const int MAX_DEPTH = DepthFrame::MAX_DEPTH;

    public:
        // Implementations of different methods

        static void correctDepthFrame_Naive(const DepthFrame& source, DepthFrame& target);
        static void correctDepthFrame_Memory(const DepthFrame& source, DepthFrame& target);
        static void correctDepthFrame_Rings(const DepthFrame& source, DepthFrame& target);

};


// Stream that attempts to correct depth maps noise and shows the result in "real time".
class DepthCorrectorStream : public AsyncStream
{
    private:
        Ptr<DataStream> base; // The depth frame source stream.

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

        // Correct a single depth frame
        static void correctDepthFrame(const DepthFrame& source, DepthFrame& target);

};


#endif
