#ifndef CALIBRATOR_H
#define CALIBRATOR_H

#pragma once

#include "globals.h"

#include "DataStream.h"

#include "Operation.h"

#include "Calibration.h"

class Calibrator : public Operation
{
    private:

        // List of streams to calibrate
        std::vector< Ptr<DataStream> > streams;

        // Stores calibration params such as: params[i] = calibration between stream[0] and stream[i]
        std::vector<ExtrinsicParams> params;

        // Number of rows and columns of the chessboard used to calibrate it
        cv::Size chessboardSize;

    public:

        Calibrator()
        {
            chessboardSize = cv::Size(9, 6);
        }

        void addStream(const Ptr<DataStream>& stream)
        {
            streams.push_back(stream);
        }

        void run() override
        {
            if (streams.size() == 1) streamCalibration();
            else if (streams.size() > 1) systemCalibration();
        }

        const std::vector< Ptr<DataStream> >& getStreams() const
        {
            return streams;
        }

        // Returns the extrinsic parameters between the first stream
        // and each other
        const std::vector<ExtrinsicParams>& getExtrinsicParams() const
        {
            return params;
        }

        bool hasExtrinsicParams() const
        {
            return !params.empty();
        }


    private:

        bool findPoints(cv::Mat& image, cv::Mat& gray, INOUT cv::vector< cv::vector<cv::Point2f> >& points);

        void generateChessboardPoints(OUT cv::Mat& points, float squareSize = 1.0f);

        //void generateChessboardPointsInVector(OUT cv::vector<cv::Point3f>& points, float squareSize = 1.0f);
        

        void streamCalibration();

        void systemCalibration();

};



#endif
