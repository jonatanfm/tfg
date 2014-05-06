#ifndef CHESSBOARDDETECTORSTREAM_H
#define CHESSBOARDDETECTORSTREAM_H

#pragma once

#include "globals.h"

#include "AsyncStream.h"

class ChessboardDetectorStream : public AsyncStream
{
    private:
        Ptr<DataStream> base;

        cv::Size chessboardSize;

    public:

        ChessboardDetectorStream(Ptr<DataStream> baseStream, int rows, int cols) :
            base(baseStream),
            chessboardSize(rows, cols)
        {
            colorFrame = newColorFrame();

            start();
        }

        std::string getName() const override
        {
            char buffer[64];
            sprintf(buffer, "Chessboard Detector (%dx%d)", chessboardSize.width, chessboardSize.height);
            return buffer;
        }

        bool isOpened() const override
        {
            return true;
        }

        void run() override
        {
            DataStream::ColorPixel* buffer = newColorFrame();
            cv::Mat image(cv::Size(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT), CV_8UC3);
            cv::Mat gray;
            while (!stopping)
            {
                base->getColorImage(image);

                cv::vector<cv::Point2f> corners;

                const int flags =
                    CV_CALIB_CB_ADAPTIVE_THRESH | // Use adaptive thresholding to convert the image to black and white, rather than a fixed threshold level(computed from the average image brightness).
                    CV_CALIB_CB_FAST_CHECK | // Normalize the image gamma with equalizeHist() before applying fixed or adaptive thresholding.
                    CV_CALIB_CB_NORMALIZE_IMAGE | // Use additional criteria (like contour area, perimeter, square-like shape) to filter out false quads extracted at the contour retrieval stage.
                    0;

                bool found = findChessboardCorners(
                    image,
                    chessboardSize,
                    OUT corners,
                    flags
                );

                if (found) {
                    cv::cvtColor(image, gray, CV_RGB2GRAY);

                    cornerSubPix(
                        gray,
                        INOUT corners,
                        cv::Size(11, 11),
                        cv::Size(-1, -1),
                        cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1)
                    );
                }

                drawChessboardCorners(image, chessboardSize, corners, found);

                Utils::rgbToColorFrame(image, buffer);
                pushFrame(buffer, nullptr, nullptr);
            }
            delete[] buffer;
        }

};


#endif
