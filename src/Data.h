//
// Defines structs for holding the data of Color, Depth and Skeleton frames,
// as well as other related data.
//
#ifndef DATA_H
#define DATA_H

#pragma once

#include "globals.h"


// A color frame pixel
union ColorPixel
{
    unsigned int color;
    unsigned char data[4];
    struct
    {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };
};

// A depth frame pixel containing the depth data, and the player index.
struct DepthPixel
{
    unsigned short playerIndex; // Player index (0 if none)
    unsigned short depth; // Depth, in millimeters.
};





// Represents a frame number. Used to check if two frames are the same
struct FrameNum
{
    friend class FixedFrameStream;

    private:
    int num;

    public:
    FrameNum() : num(0) { }

    FrameNum(const FrameNum& other) : num(other.num) { }

    inline bool operator==(const FrameNum& other) const
    {
        return num == other.num;
    }

    inline bool operator!=(const FrameNum& other) const
    {
        return num != other.num;
    }

    inline void operator++()
    {
        ++num;
    }

    inline int getNumber() const
    {
        return num;
    }
};


// A Kinect color frame.
struct ColorFrame
{
    static const int WIDTH = 640;
    static const int HEIGHT = 480;

    static const int SIZE = WIDTH * HEIGHT;
    static const unsigned int BYTES = SIZE * sizeof(ColorPixel);  // Size in bytes of a Color frame


    ColorPixel pixels[SIZE];

    inline void clear()
    {
        memset(pixels, 0, sizeof(pixels));
    }

    inline ColorFrame& operator=(const ColorFrame& src)
    {
        memcpy(pixels, src.pixels, sizeof(pixels));
        return *this;
    }

    inline ColorPixel& operator()(int x, int y)
    {
        return pixels[y * WIDTH + x];
    }

    inline const ColorPixel& operator()(int x, int y) const
    {
        return pixels[y * WIDTH + x];
    }

};


// A Kinect depth frame.
struct DepthFrame
{
    static const int WIDTH = 640;
    static const int HEIGHT = 480;

    static const int SIZE = WIDTH * HEIGHT;
    static const unsigned int BYTES = SIZE * sizeof(DepthPixel);  // Size in bytes of a Depth frame

    // Minimum reliable depth value
    static const int MIN_DEPTH = NUI_IMAGE_DEPTH_MINIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

    // Maximum reliable depth value
    static const int MAX_DEPTH = NUI_IMAGE_DEPTH_MAXIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;


    DepthPixel pixels[SIZE];

    inline void clear()
    {
        memset(pixels, 0, sizeof(pixels));
    }

    inline DepthFrame& operator=(const DepthFrame& src)
    {
        memcpy(pixels, src.pixels, sizeof(pixels));
        return *this;
    }

    inline DepthPixel& operator()(int x, int y)
    {
        return pixels[y * WIDTH + x];
    }

    inline const DepthPixel& operator()(int x, int y) const
    {
        return pixels[y * WIDTH + x];
    }

};


// A Kinect skeleton(s) frame.
struct SkeletonFrame
{
    NUI_SKELETON_FRAME frame;


    inline SkeletonFrame()
    {
        frame.dwFrameNumber = 0;
    }

    inline bool isValid() const
    {
        return frame.dwFrameNumber != 0;
    }

    inline SkeletonFrame& operator=(const SkeletonFrame& src)
    {
        memcpy(&frame, &src.frame, sizeof(frame));
        return *this;
    }

    inline void clear()
    {
        memset(&frame, 0, sizeof(frame));
    }
};


// Stores the trajectory of one or more skeleton joints over time.
struct SkeletonTrajectory
{
    private:
        SkeletonTrajectory(const SkeletonTrajectory&);
        SkeletonTrajectory& operator=(const SkeletonTrajectory&);
    public:
    
        struct Point
        {
            union
            {
                struct { float x, y, z; };
                float position[3];
            };
            NUI_SKELETON_POSITION_TRACKING_STATE state;
        };

        int numFrames;
        std::vector<NUI_SKELETON_POSITION_INDEX> joints;
        
        Point* points;
        
        
        SkeletonTrajectory() : numFrames(0), points(nullptr) { }
        
        ~SkeletonTrajectory()
        {
            if (points != nullptr) delete[] points;
        }
        
        inline Point& getPoint(int jointIndex, int frame)
        {
            return points[jointIndex*joints.size() + frame];
        }
        
        inline const Point& getPoint(int jointIndex, int frame) const
        {
            return points[jointIndex*joints.size() + frame];
        }
        
};




#endif