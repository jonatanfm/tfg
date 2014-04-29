#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#pragma once

#include "globals.h"

typedef unsigned int Texture;

struct Point2D
{
    float x, y;
};

class RenderUtils
{
    private:
        RenderUtils();
        ~RenderUtils();

    public:

        static Texture createTexture(int width, int height);

        static void drawRect(float x, float y, float w, float h, float tx = 0.0f, float ty = 0.0f, float tw = 1.0f, float th = 1.0f);

        static void drawPoint(float x, float y, float radius);

        static void drawLine(float x1, float y1, float x2, float y2, float lineWidth = 1.0f);


        static void setColor(float r, float g, float b, float a = 1.0f);

        static void setColor(float* color);

        static void setTexture(Texture tex);


        static void drawBone(const NUI_SKELETON_DATA& skel, Point2D* points, NUI_SKELETON_POSITION_INDEX joint1, NUI_SKELETON_POSITION_INDEX joint2);

        static void drawSkeleton(const NUI_SKELETON_DATA& skel, bool colorFrame);

        static void drawSkeletons(const NUI_SKELETON_FRAME& frame, bool colorFrame);
};

#endif
