#ifndef CALLIBRATOR_H
#define CALLIBRATOR_H

#pragma once

#include "globals.h"

#include "DataStream.h"

class Calibrator : private QThread
{
    public:

        struct IntrinsicParams
        {
            cv::Mat cameraMatrix;
            cv::Mat distCoeffs;
            double reprojectionError; // Root mean square, should be between 0.1 and 1.0 for a good calibration
        };

        struct ExtrinsicParams
        {
            cv::Mat R; // Output rotation matrix between the 1st and the 2nd camera coordinate systems
            cv::Mat T; // Output translation vector between the coordinate systems of the cameras
            cv::Mat E; // Output essential matrix
            cv::Mat F; // Output fundamental matrix
        };

    private:

        // List of streams to calibrate
        std::vector< Ptr<DataStream> > streams;

        // Stores calibration params such as: params[i] = calibration between stream[0] and stream[i]
        std::vector<ExtrinsicParams> params;

        IntrinsicParams intrinsic;

        // Number of rows and columns of the chessboard used to calibrate it
        cv::Size chessboardSize;

    public:

        Calibrator()
        {
            chessboardSize = cv::Size(9, 6);
        }

        ~Calibrator()
        {
            if (isRunning()) {
                wait();
            }
        }

        void addStream(const Ptr<DataStream>& stream)
        {
            streams.push_back(stream);
        }

        bool calibrateStream()
        {
            if (streams.size() != 1) return false;
            if (!isRunning()) start();
            return true;
        }

        bool calibrateSystem()
        {
            if (streams.size() < 2) return false;
            if (!isRunning()) start();
            return true;
        }


    private:

        bool findPoints(cv::Mat& image, cv::Mat& gray, INOUT cv::vector< cv::vector<cv::Point2f> >& points);

        void generateChessboardPoints(OUT cv::vector<cv::Point3f>& points, float squareSize = 1.0f);

        void streamCalibration();

        void systemCalibration();

    protected:
        void run() override
        {
            if (streams.size() == 1) streamCalibration();
            else if (streams.size() > 2) systemCalibration();
        }

};



#endif
