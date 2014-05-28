#ifndef CALIBRATION_H
#define CALIBRATION_H

#pragma once

#include "globals.h"


// Intrinsic callibration parameters for the color camera.
struct IntrinsicParams
{
    cv::Mat cameraMatrix;
    cv::Mat distCoeffs;
    double reprojectionError; // Root mean square, should be between 0.1 and 1.0 for a good calibration
};


// Extrinsic callibration parameters between a set of color cameras.
struct ExtrinsicParams
{
    cv::Mat R; // Rotation matrix between the 1st and the 2nd stream cameras coordinate systems
    cv::Mat T; // Translation vector between the coordinate systems of the cameras
    cv::Mat E; // Essential matrix
    cv::Mat F; // Fundamental matrix

    // Get the parameters to transform from the target stream to the base stream
    ExtrinsicParams invert() const
    {
        ExtrinsicParams dest;
        dest.R = R.inv();
        dest.T = -T;
        dest.E = E.inv(); // TODO: is correct?
        dest.F = F.inv(); // TODO: is correct?
        return dest;
    }


    cv::Mat getTransformMatrix() const
    {
        cv::Mat mat = cv::Mat::eye(4, 4, CV_32F);
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                mat.at<float>(i, j) = R.at<double>(i, j);
            }
        }
        mat.at<float>(0, 3) = T.at<double>(0);
        mat.at<float>(1, 3) = T.at<double>(1);
        mat.at<float>(2, 3) = T.at<double>(2);
        return mat;
    }

};


// Holds the state of one or more multi-device callibrations
class SystemCalibration
{
    private:
        typedef std::map< std::pair<int, int>, ExtrinsicParams > Calibrations;

        Calibrations calibrations;

    public:

        void reset()
        {
            calibrations.clear();
        }

        void add(int baseStreamIdx, int targetStreamIdx, const ExtrinsicParams& params)
        {
            calibrations[std::make_pair(baseStreamIdx, targetStreamIdx)] = params;
        }

        std::vector< std::pair<int, ExtrinsicParams> > getCalibratedWith(int base) const
        {
            std::vector< std::pair<int, ExtrinsicParams> > v;
            for (auto it = calibrations.begin(); it != calibrations.end(); ++it) {
                auto& idxs = it->first;
                if (idxs.first == base) {
                    v.push_back(std::make_pair(idxs.second, it->second));
                }
                else if (idxs.second == base) {
                    v.push_back(std::make_pair(idxs.first, it->second.invert()));
                }
            }
            return v;
        }

};


#endif
