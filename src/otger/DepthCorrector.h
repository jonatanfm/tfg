#ifndef DEPTHCORRECTOR_H
#define DEPTHCORRECTOR_H

#pragma once

#include "../globals.h"

#include "../AsyncStream.h"


// Base class for depth correction algorithms
// (They operate like Functors)
class DepthCorrectionAlgorithm
{
    protected:
        static const int MIN_DEPTH = DepthFrame::MIN_DEPTH;
        static const int MAX_DEPTH = DepthFrame::MAX_DEPTH;

    public:
        DepthCorrectionAlgorithm() { }
        virtual ~DepthCorrectionAlgorithm() { }

        virtual void correct(const DepthFrame& source, DepthFrame& target) = 0;

        inline void operator()(const DepthFrame& source, DepthFrame& target)
        {
            correct(source, target);
        }

};


class DepthCorrectionAlgorithm_Naive : public DepthCorrectionAlgorithm
{
    public:
        void correct(const DepthFrame& source, DepthFrame& target) override;
};

class DepthCorrectionAlgorithm_Rings : public DepthCorrectionAlgorithm
{
    public:
        void correct(const DepthFrame& source, DepthFrame& target) override;
};

class DepthCorrectionAlgorithm_Memory: public DepthCorrectionAlgorithm
{
    private:
        DepthFrame memoryFrame;

    public:
        DepthCorrectionAlgorithm_Memory()
        {
            memoryFrame.clear();
        }

        void correct(const DepthFrame& source, DepthFrame& target) override;
};


// Stream that attempts to correct depth maps noise and shows the result in "real time".
class DepthCorrectorStream : public AsyncStream
{
    private:
        Ptr<DataStream> base; // The depth frame source stream.

        unique_ptr<DepthCorrectionAlgorithm> corrector;

    public:

        DepthCorrectorStream(Ptr<DataStream> baseStream, int algorithm = 0) :
            base(baseStream),
            corrector()
        {
            depthFrame = new DepthFrame();

            DepthCorrectionAlgorithm* alg;
            switch (algorithm) {
                case 1: alg = new DepthCorrectionAlgorithm_Rings(); break;
                case 2: alg = new DepthCorrectionAlgorithm_Memory(); break;
                default: alg = new DepthCorrectionAlgorithm_Naive(); break;
            }
            corrector.reset(alg);

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
    
                corrector->correct(*input, *output);

                pushFrame(nullptr, output, nullptr);
            }
            delete input;
            delete output;
        }

        // Correct a single depth frame
        static void correctDepthFrame(const DepthFrame& source, DepthFrame& target);

};


#endif
