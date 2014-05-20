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

        static void colorFrameToRgb(const ColorFrame& frame, cv::Mat& image)
        {
            const ColorPixel* src = &frame.pixels[0];
            const ColorPixel* end = &frame.pixels[0] + ColorFrame::WIDTH * ColorFrame::HEIGHT;

            unsigned char* dest = image.data;
            while (src < end) {
                *dest++ = src->b;
                *dest++ = src->g;
                *dest++ = src->r;
                ++src;
            }
        }

        static void rgbToColorFrame(const cv::Mat& image, ColorFrame& frame)
        {
            const unsigned char* src = image.data;
            ColorPixel* dest = &frame.pixels[0];
            ColorPixel* end = &frame.pixels[0] + ColorFrame::WIDTH * ColorFrame::HEIGHT;

            while (dest < end) {
                dest->b = *src++;
                dest->g = *src++;
                dest->r = *src++;
                ++dest;
            }
        }


        static void depthFrameToRgb(const DepthFrame& frame, cv::Mat& image)
        {
            const DepthPixel* src = &frame.pixels[0];
            const DepthPixel* end = &frame.pixels[0] + DepthFrame::WIDTH * DepthFrame::HEIGHT;

            unsigned char* dest = image.data;
            while (src < end) {
                *dest++ = (src->depth) >> 8;
                *dest++ = (src->depth) & 0x00FF;
                *dest++ = src->playerIndex;
                ++src;
            }
        }

        static void rgbToDepthFrame(const cv::Mat& image, DepthFrame& frame)
        {
            DepthPixel* dest = &frame.pixels[0];
            //if (image.isContinuous()) {
            DepthPixel* end = &frame.pixels[0] + DepthFrame::WIDTH * DepthFrame::HEIGHT;
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
