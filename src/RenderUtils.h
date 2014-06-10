#ifndef RENDERUTILS_H
#define RENDERUTILS_H

#pragma once

#include "globals.h"

// Define "glCheckError()" as a -function- to check if OpenGL has emitted any error, and if so, report it.
#ifdef _DEBUG
    #define glCheckError() \
    { \
        GLenum error = glGetError(); \
        if (error != GL_NO_ERROR) { \
            qDebug("%s(%d): OpenGL error \"%s\" [%d] at '%s'!\n", __FILE__, __LINE__, gluErrorString(error), error, __FUNCTION__); \
            DebugBreak(); \
        } \
    }
#else
    #define glCheckError()
#endif


// OpenGL texture (typedef'd to make more readable passing and using it)
typedef GLuint Texture;


// A simple bidimensional point
struct Point2D
{
    float x, y;

    inline Point2D() { }
    inline Point2D(float x, float y) : x(x), y(y) { }
};

// A simple tridimensional point
struct Point3D
{
    float x, y, z;

    inline Point3D() { }
    inline Point3D(float x, float y, float z) : x(x), y(y), z(z) { }
};


// Static class that provides rendering utility functions
class RenderUtils
{
    private:
        RenderUtils();

    public:

        // Creates a new empty texture with the given size and internal format.
        static Texture createTexture(int width, int height, GLint internalFormat = GL_RGBA, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

        // Creates a new texture from the given image.
        static Texture createTexture(const cv::Mat& image, GLint internalFormat = GL_RGBA);


        // Draws a (textured) 2D rect in the given (x,y) and with the given width and height.
        // Optionally accept texture coordinates.
        static void drawRect(float x, float y, float w, float h, float tx = 0.0f, float ty = 0.0f, float tw = 1.0f, float th = 1.0f);

        // Draws a 2D point with the given radius.
        static void drawPoint(const Point2D& p, float radius);

        // Draws a 3D point with the given radius.
        static void drawPoint(const Point3D& p, float radius);

        // Draws a 2D line between two points with the given width.
        static void drawLine(const Point2D& p1, const Point2D& p2, float lineWidth = 1.0f);

        // Draws a 3D line between two points with the given width.
        static void drawLine(const Point3D& p1, const Point3D& p2, float lineWidth = 1.0f);

        // Draws a 3D cube centered at the origin and with the given half-extent values for width, height and depth.
        static void drawCube(float szX, float szY, float szZ);


        // Sets the current color
        static void setColor(float r, float g, float b, float a = 1.0f);

        // Sets the current color to the given float[4]
        static void setColor(float* color);

        // Sets the current texture
        static void setTexture(Texture tex);


        // Function type for functions converting skeleton points to the target space coordinates.
        typedef Point3D (*SkeletonPointConverter)(const Vector4&);


        // Draws a single skeleton bone, as specified by two joints indices
        static void drawBone(const NUI_SKELETON_DATA& skel, Point3D* points, NUI_SKELETON_POSITION_INDEX joint1, NUI_SKELETON_POSITION_INDEX joint2);

        // Draws a single skeleton using the given coordinate converter.
        static void drawSkeleton(const NUI_SKELETON_DATA& skel, SkeletonPointConverter pointConverter);

        // Draws all skeletons on a skeleton frame, for a 2D color or depth frame.
        static void drawSkeletons(const NUI_SKELETON_FRAME& frame, bool colorFrame);

        // Draws all skeletons on a skeleton frame, for a 3D scene.
        static void drawSkeletons3D(const NUI_SKELETON_FRAME& frame);
};

class TextureManager
{
    private:
        std::map<std::string, Texture> textures;

    public:

        TextureManager() { }

        ~TextureManager()
        {
            clear();
        }

        void clear()
        {
            for (auto it = textures.begin(); it != textures.end(); ++it)
            {
                glDeleteTextures(1, &it->second);
            }
            textures.clear();
        }

        Texture getTexture(const std::string& file)
        {
            auto it = textures.find(file);
            if (it != textures.end()) return it->second;

            cv::Mat img = cv::imread(file);
            Texture tex = RenderUtils::createTexture(img);
            textures[file] = tex;
            return tex;
        }
};

#endif
