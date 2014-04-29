#include "RenderUtils.h"

#include "globals.h"

Texture RenderUtils::createTexture(int width, int height)
{
    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

void RenderUtils::drawRect(float x, float y, float w, float h, float tx, float ty, float tw, float th)
{
    glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(tx, ty);
        glVertex2f(x, y);

        glTexCoord2f(tx, ty + th);
        glVertex2f(x, y + h);

        glTexCoord2f(tx + tw, ty);
        glVertex2f(x + w, y);

        glTexCoord2f(tx + tw, ty + th);
        glVertex2f(x + w, y + h);
    glEnd();
}


void RenderUtils::drawPoint(float x, float y, float radius)
{
    drawRect(x - radius, y - radius, 2.0f * radius, 2.0f * radius);
}

void RenderUtils::drawLine(float x1, float y1, float x2, float y2, float lineWidth)
{
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
    glEnd();
}


void RenderUtils::setColor(float* color)
{
    glColor4fv(color);
}

void RenderUtils::setColor(float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
}

void RenderUtils::setTexture(Texture tex)
{
    if (tex == 0) {
        glDisable(GL_TEXTURE_2D);
    }
    else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex);
    }
}


#pragma region Skeleton Rendering

Point2D skeletonToDepthFrame(Vector4 skelPoint, int w, int h)
{
    LONG x, y;
    USHORT depth;
    NuiTransformSkeletonToDepthImage(skelPoint, &x, &y, &depth, NUI_IMAGE_RESOLUTION_640x480);
    Point2D pt;
    pt.x = static_cast<float>(x * 640) / w;
    pt.y = static_cast<float>(y * 480) / h;
    return pt;
}

Point2D skeletonToColorFrame(Vector4 skelPoint, int w, int h)
{
    LONG x, y;
    USHORT depth;
    NuiTransformSkeletonToDepthImage(skelPoint, &x, &y, &depth, NUI_IMAGE_RESOLUTION_640x480);
    Point2D pt;
    pt.x = static_cast<float>(x * 640) / w;
    pt.y = static_cast<float>(y * 480) / h;

    // TODO
    /*NuiImageGetColorPixelCoordinatesFromDepthPixel(
        NUI_IMAGE_RESOLUTION_640x480,
        nullptr,
        pt.x,
        pt.y,
        depth,
        &pt.x,
        &pt.p
    );*/
    
    return pt;
}

static float COLOR_BONE_TRACKED[] = { 0.0f, 0.5f, 0.0f, 1.0f };
static float COLOR_BONE_INFERRED[] = { 0.5f, 0.0f, 0.0f, 1.0f };

void RenderUtils::drawBone(const NUI_SKELETON_DATA& skel, Point2D* points, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1)
{
    NUI_SKELETON_POSITION_TRACKING_STATE
        s0 = skel.eSkeletonPositionTrackingState[joint0],
        s1 = skel.eSkeletonPositionTrackingState[joint1];

    if (s0 == NUI_SKELETON_POSITION_NOT_TRACKED ||
        s1 == NUI_SKELETON_POSITION_NOT_TRACKED)
    {
        return;
    }

    if (s0 == NUI_SKELETON_POSITION_INFERRED &&
        s1 == NUI_SKELETON_POSITION_INFERRED)
    {
        return;
    }

    if (s0 == NUI_SKELETON_POSITION_TRACKED &&
        s1 == NUI_SKELETON_POSITION_TRACKED)
    {
        RenderUtils::setColor(COLOR_BONE_TRACKED);
    }
    else {
        RenderUtils::setColor(COLOR_BONE_INFERRED);
    }

    Point2D& p0 = points[joint0];
    Point2D& p1 = points[joint1];

    RenderUtils::drawLine(p0.x, p0.y, p1.x, p1.y);
}

void RenderUtils::drawSkeleton(const NUI_SKELETON_DATA& skel, bool inColorFrame)
{
    auto convertCoordinates = inColorFrame ? skeletonToColorFrame : skeletonToDepthFrame;

    Point2D points[NUI_SKELETON_POSITION_COUNT];
    for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i) {
        points[i] = convertCoordinates(skel.SkeletonPositions[i], 640, 480);
    }

    // Torso
    drawBone(skel, points, NUI_SKELETON_POSITION_HEAD, NUI_SKELETON_POSITION_SHOULDER_CENTER);
    drawBone(skel, points, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_LEFT);
    drawBone(skel, points, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SHOULDER_RIGHT);
    drawBone(skel, points, NUI_SKELETON_POSITION_SHOULDER_CENTER, NUI_SKELETON_POSITION_SPINE);
    drawBone(skel, points, NUI_SKELETON_POSITION_SPINE, NUI_SKELETON_POSITION_HIP_CENTER);
    drawBone(skel, points, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_LEFT);
    drawBone(skel, points, NUI_SKELETON_POSITION_HIP_CENTER, NUI_SKELETON_POSITION_HIP_RIGHT);

    // Left Arm
    drawBone(skel, points, NUI_SKELETON_POSITION_SHOULDER_LEFT, NUI_SKELETON_POSITION_ELBOW_LEFT);
    drawBone(skel, points, NUI_SKELETON_POSITION_ELBOW_LEFT, NUI_SKELETON_POSITION_WRIST_LEFT);
    drawBone(skel, points, NUI_SKELETON_POSITION_WRIST_LEFT, NUI_SKELETON_POSITION_HAND_LEFT);

    // Right Arm
    drawBone(skel, points, NUI_SKELETON_POSITION_SHOULDER_RIGHT, NUI_SKELETON_POSITION_ELBOW_RIGHT);
    drawBone(skel, points, NUI_SKELETON_POSITION_ELBOW_RIGHT, NUI_SKELETON_POSITION_WRIST_RIGHT);
    drawBone(skel, points, NUI_SKELETON_POSITION_WRIST_RIGHT, NUI_SKELETON_POSITION_HAND_RIGHT);

    // Left Leg
    drawBone(skel, points, NUI_SKELETON_POSITION_HIP_LEFT, NUI_SKELETON_POSITION_KNEE_LEFT);
    drawBone(skel, points, NUI_SKELETON_POSITION_KNEE_LEFT, NUI_SKELETON_POSITION_ANKLE_LEFT);
    drawBone(skel, points, NUI_SKELETON_POSITION_ANKLE_LEFT, NUI_SKELETON_POSITION_FOOT_LEFT);

    // Right Leg
    drawBone(skel, points, NUI_SKELETON_POSITION_HIP_RIGHT, NUI_SKELETON_POSITION_KNEE_RIGHT);
    drawBone(skel, points, NUI_SKELETON_POSITION_KNEE_RIGHT, NUI_SKELETON_POSITION_ANKLE_RIGHT);
    drawBone(skel, points, NUI_SKELETON_POSITION_ANKLE_RIGHT, NUI_SKELETON_POSITION_FOOT_RIGHT);

    // Joints
    for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i) {
        NUI_SKELETON_POSITION_TRACKING_STATE state = skel.eSkeletonPositionTrackingState[i];
        if (state == NUI_SKELETON_POSITION_TRACKED) {
            RenderUtils::setColor(COLOR_BONE_TRACKED);
        }
        else if (state == NUI_SKELETON_POSITION_INFERRED) {
            RenderUtils::setColor(COLOR_BONE_INFERRED);
        }
        RenderUtils::drawPoint(points[i].x, points[i].y, 3.0f);
    }
}

void RenderUtils::drawSkeletons(const NUI_SKELETON_FRAME& frame, bool inColorFrame)
{
    for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE state = frame.SkeletonData[i].eTrackingState;
        if (NUI_SKELETON_TRACKED == state) {
            drawSkeleton(frame.SkeletonData[i], inColorFrame);
        }
        else if (NUI_SKELETON_POSITION_ONLY == state) {
            Point2D pos = skeletonToDepthFrame(frame.SkeletonData[i].Position, 640, 480);
            RenderUtils::setColor(1.0f, 0.0f, 0.0f);
            RenderUtils::drawPoint(pos.x, pos.y, 5.0f);
        }
    }
}

#pragma endregion
