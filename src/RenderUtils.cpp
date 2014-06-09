#include "RenderUtils.h"

#include "globals.h"

Texture RenderUtils::createTexture(int width, int height, GLint internalFormat, GLenum format, GLenum type)
{
    GLuint tex;
    glGenTextures(1, &tex);

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);

    return tex;
}

Texture RenderUtils::createTexture(const cv::Mat& img, GLint internalFormat)
{
    GLuint tex;
    glGenTextures(1, &tex);

    GLenum format, type;

    switch (img.channels()) {
        case 1: format = GL_RED; break;
        case 2: format = GL_RG; break;
        case 3: format = GL_BGR; break;
        default: format = GL_BGRA;
    }

    switch (img.depth()) {
        case CV_8S: type = GL_BYTE; break;
        case CV_16U: type = GL_UNSIGNED_SHORT; break;
        case CV_16S: type = GL_SHORT; break;
        case CV_32S: type = GL_INT; break;
        case CV_32F: type = GL_FLOAT; break;
        default: type = GL_UNSIGNED_BYTE;
    }

    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, img.cols, img.rows, 0, format, type, (GLvoid*)img.data);

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


void RenderUtils::drawPoint(const Point2D& p, float radius)
{
    drawRect(p.x - radius, p.y - radius, 2.0f * radius, 2.0f * radius);
}

void RenderUtils::drawPoint(const Point3D& p, float radius)
{
	glPointSize(radius);
    glBegin(GL_POINTS);
        glVertex3f(p.x, p.y, p.z);
        //glVertex3f(p.x, p.y, p.z);
    glEnd();
}

void RenderUtils::drawLine(const Point2D& p1, const Point2D& p2, float lineWidth)
{
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
        glVertex2f(p1.x, p1.y);
        glVertex2f(p2.x, p2.y);
    glEnd();
}

void RenderUtils::drawLine(const Point3D& p1, const Point3D& p2, float lineWidth)
{
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
        glVertex3f(p1.x, p1.y, p1.z);
        glVertex3f(p2.x, p2.y, p2.z);
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

void RenderUtils::drawCube(float szX, float szY, float szZ)
{
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-szX, -szY, szZ);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(szX, -szY, szZ);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(szX, szY, szZ);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-szX, szY, szZ);

        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-szX, -szY, -szZ);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-szX, szY, -szZ);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(szX, szY, -szZ);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(szX, -szY, -szZ);

        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-szX, szY, -szZ);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-szX, szY, szZ);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(szX, szY, szZ);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(szX, szY, -szZ);

        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-szX, -szY, -szZ);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(szX, -szY, -szZ);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(szX, -szY, szZ);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-szX, -szY, szZ);

        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(szX, -szY, -szZ);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(szX, szY, -szZ);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(szX, szY, szZ);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(szX, -szY, szZ);

        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-szX, -szY, -szZ);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-szX, -szY, szZ);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-szX, szY, szZ);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-szX, szY, -szZ);
    glEnd();
}


#pragma region Skeleton Rendering

Point3D skeletonToDepthFrame(const Vector4& skelPoint)
{
    LONG x, y;
    USHORT depth;
    NuiTransformSkeletonToDepthImage(skelPoint, &x, &y, &depth, NUI_IMAGE_RESOLUTION_640x480);
    //return Point3D(float(x * 640) / 640, float(y * 480) / 480, 0.0f);
    return Point3D(float(x), float(y), 0.0f);
}

Point3D skeletonToColorFrame(const Vector4& skelPoint)
{
    LONG x, y;
    USHORT depth;
    NuiTransformSkeletonToDepthImage(skelPoint, &x, &y, &depth, NUI_IMAGE_RESOLUTION_640x480);

    NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
        NUI_IMAGE_RESOLUTION_640x480,
        NUI_IMAGE_RESOLUTION_640x480,
        nullptr,
        x,
        y,
        depth,
        &x,
        &y
    );

    return Point3D(float(x), float(y), 0.0f);
}

Point3D skeletonTo3D(const Vector4& skelPoint)
{
    return Point3D(-skelPoint.x, skelPoint.y, skelPoint.z);
}

static float COLOR_BONE_TRACKED[] = { 0.0f, 0.5f, 0.0f, 1.0f };
static float COLOR_BONE_INFERRED[] = { 0.5f, 0.0f, 0.0f, 1.0f };

void RenderUtils::drawBone(const NUI_SKELETON_DATA& skel, Point3D* points, NUI_SKELETON_POSITION_INDEX joint0, NUI_SKELETON_POSITION_INDEX joint1)
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

    Point3D& p0 = points[joint0];
    Point3D& p1 = points[joint1];

    RenderUtils::drawLine(p0, p1);
}

void RenderUtils::drawSkeleton(const NUI_SKELETON_DATA& skel, SkeletonPointConverter pointConverter)
{
    Point3D points[NUI_SKELETON_POSITION_COUNT];
    for (int i = 0; i < NUI_SKELETON_POSITION_COUNT; ++i) {
        points[i] = pointConverter(skel.SkeletonPositions[i]);
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
		else if (state == NUI_SKELETON_POSITION_NOT_TRACKED) {
			qDebug()<<"hola"<<endl;
            RenderUtils::setColor(COLOR_BONE_TRACKED);
        }
		
        RenderUtils::drawPoint(points[i], 3.0f);
    }
}

void RenderUtils::drawSkeletons(const NUI_SKELETON_FRAME& frame, bool inColorFrame)
{
    SkeletonPointConverter pointConverter = inColorFrame ? skeletonToColorFrame : skeletonToDepthFrame;
    for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE state = frame.SkeletonData[i].eTrackingState;
        if (NUI_SKELETON_TRACKED == state) {
            drawSkeleton(frame.SkeletonData[i], pointConverter);
        }
        else if (NUI_SKELETON_POSITION_ONLY == state) {
            Point3D pos = pointConverter(frame.SkeletonData[i].Position);
            RenderUtils::setColor(1.0f, 0.0f, 0.0f);
            RenderUtils::drawPoint(pos, 5.0f);
        }
    }
}


void RenderUtils::drawSkeletons3D(const NUI_SKELETON_FRAME& frame)
{
    for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
        NUI_SKELETON_TRACKING_STATE state = frame.SkeletonData[i].eTrackingState;
        if (NUI_SKELETON_TRACKED == state) {
            drawSkeleton(frame.SkeletonData[i], skeletonTo3D);
        }
        else if (NUI_SKELETON_POSITION_ONLY == state) {
            Point3D pos = skeletonTo3D(frame.SkeletonData[i].Position);
            RenderUtils::setColor(1.0f, 0.0f, 0.0f);
            RenderUtils::drawPoint(pos, 5.0f);
        }
    }
}


#pragma endregion
