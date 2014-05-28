#ifndef UTILS_H
#define UTILS_H

#pragma once

#include <qwaitcondition.h>

#include "globals.h"

#include "DataStream.h"

// Static class defining miscellaneous utility functions
class Utils
{
    private:
        Utils();

    public:

        // Converts a cv::Mat to a string (for debugging)
        template<typename T>
        static std::string matToString(const cv::Mat& mat)
        {
            std::string s = "{\n";
            char buffer[32];
            for (int i = 0; i < mat.rows; ++i) {
                for (int j = 0; j < mat.cols; ++j) {
                    sprintf(buffer, "%.2f", mat.at<T>(i, j));
                    s += "\t" + std::string(buffer);
                }
                s += "\n";
            }
            return s + "}\n";
        }


        // Encode a ColorFrame into a cv::Mat color image
        static void colorFrameToRgb(const ColorFrame& frame, cv::Mat& image)
        {
            const ColorPixel* src = frame.pixels;
            const ColorPixel* end = frame.pixels + ColorFrame::SIZE;
            unsigned char* dest = image.data;

            while (src < end) {
                *dest++ = src->b;
                *dest++ = src->g;
                *dest++ = src->r;
                ++src;
            }
        }

        // Decode a ColorFrame from a cv::Mat color image
        static void rgbToColorFrame(const cv::Mat& image, ColorFrame& frame)
        {
            const unsigned char* src = image.data;
            ColorPixel* dest = frame.pixels;
            ColorPixel* end = frame.pixels + ColorFrame::SIZE;

            while (dest < end) {
                dest->b = *src++;
                dest->g = *src++;
                dest->r = *src++;
                ++dest;
            }
        }

        // Encode a DepthFrame into a cv::Mat color image
        static void depthFrameToRgb(const DepthFrame& frame, cv::Mat& image)
        {
            const DepthPixel* src = frame.pixels;
            const DepthPixel* end = frame.pixels + DepthFrame::SIZE;

            unsigned char* dest = image.data;
            while (src < end) {
                *dest++ = (src->depth) >> 8;
                *dest++ = (src->depth) & 0x00FF;
                *dest++ = src->playerIndex;
                ++src;
            }
        }

        // Decode a DepthFrame from a cv::Mat color image
        static void rgbToDepthFrame(const cv::Mat& image, DepthFrame& frame)
        {
            DepthPixel* dest = frame.pixels;
            //if (image.isContinuous()) {
            DepthPixel* end = frame.pixels + DepthFrame::SIZE;
            const unsigned char* src = image.data;
            while (dest < end) {
                dest->depth = ((*src) << 8) | *(src + 1);
                dest->playerIndex = *(src + 2);
                src += 3;
                ++dest;
            }
            /*}
            else {
                for (int i = 0; i < DepthFrame::HEIGHT; ++i) {
                    const unsigned char* src = image.ptr(i);
                    for (int j = 0; j < DepthFrame::WIDTH; ++j) {
                        dest->depth = ((*src) << 8) | *(src + 1);
                        dest->playerIndex = *(src + 2);
                        src += 3;
                        ++dest;
                    }
                }
            }*/
        }

};

#endif
