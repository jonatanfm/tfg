#ifndef UTILS_H
#define UTILS_H

#pragma once

#include <qwaitcondition.h>

#include "globals.h"

#include "DataStream.h"

class Utils
{
    private:
        Utils();
        ~Utils();

    public:

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

        static void colorFrameToRgb(const DataStream::ColorPixel* frame, cv::Mat& image)
        {
            const DataStream::ColorPixel* src = frame;
            const DataStream::ColorPixel* end = frame + COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT;

            unsigned char* dest = image.data;
            while (src < end) {
                *dest++ = src->b;
                *dest++ = src->g;
                *dest++ = src->r;
                ++src;
            }
        }

        static void rgbToColorFrame(const cv::Mat& image, DataStream::ColorPixel* frame)
        {
            const unsigned char* src = image.data;
            DataStream::ColorPixel* dest = frame;
            DataStream::ColorPixel* end = frame + COLOR_FRAME_WIDTH * COLOR_FRAME_HEIGHT;

            while (dest < end) {
                dest->b = *src++;
                dest->g = *src++;
                dest->r = *src++;
                ++dest;
            }
        }


        static void depthFrameToRgb(const DataStream::DepthPixel* frame, cv::Mat& image)
        {
            const DataStream::DepthPixel* src = frame;
            const DataStream::DepthPixel* end = frame + DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT;

            unsigned char* dest = image.data;
            while (src < end) {
                *dest++ = (src->depth) >> 8;
                *dest++ = (src->depth) & 0x00FF;
                *dest++ = src->playerIndex;
                ++src;
            }
        }

        static void rgbToDepthFrame(const cv::Mat& image, DataStream::DepthPixel* frame)
        {
            const unsigned char* src = image.data;
            DataStream::DepthPixel* dest = frame;
            DataStream::DepthPixel* end = frame + DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT;

            while (dest < end) {
                dest->depth = ((*src) << 8) | *(src + 1);
                dest->playerIndex = *(src + 2);
                src += 3;
                ++dest;
            }
        }




        /*static float depthToMeters(int depth)
        {
            if (depth < 2047)
            {
                return 1.0f / (depth * -0.0030711016f + 3.3309495161f);
            }
            return 0.0f;
        }*/

};

#endif
