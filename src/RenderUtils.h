#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#pragma once

#include "globals.h"

typedef unsigned int Texture;

struct Point2D
{
    float x, y;

    inline Point2D() { }
    inline Point2D(float x, float y) : x(x), y(y) { }
};

struct Point3D
{
    float x, y, z;

    inline Point3D() { }
    inline Point3D(float x, float y, float z) : x(x), y(y), z(z) { }
};

class RenderUtils
{
    private:
        RenderUtils();
        ~RenderUtils();

    public:

        static Texture createTexture(int width, int height);

        static void drawRect(float x, float y, float w, float h, float tx = 0.0f, float ty = 0.0f, float tw = 1.0f, float th = 1.0f);

        static void drawPoint(const Point2D& p, float radius);

        static void drawPoint(const Point3D& p, float radius);

        static void drawLine(const Point2D& p1, const Point2D& p2, float lineWidth = 1.0f);

        static void drawLine(const Point3D& p1, const Point3D& p2, float lineWidth = 1.0f);


        static void setColor(float r, float g, float b, float a = 1.0f);

        static void setColor(float* color);

        static void setTexture(Texture tex);


        typedef Point3D (*SkeletonPointConverter)(const Vector4&);

        static void drawBone(const NUI_SKELETON_DATA& skel, Point3D* points, NUI_SKELETON_POSITION_INDEX joint1, NUI_SKELETON_POSITION_INDEX joint2);

        static void drawSkeleton(const NUI_SKELETON_DATA& skel, SkeletonPointConverter pointConverter);

        static void drawSkeletons(const NUI_SKELETON_FRAME& frame, bool colorFrame);

        static void drawSkeletons3D(const NUI_SKELETON_FRAME& frame);
};

#endif
