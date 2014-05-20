#ifndef WIDGETDEPTHVIEW_H
#define WIDGETDEPTHVIEW_H

#pragma once

#include "globals.h"

#include "RenderUtils.h"
#include "DataStream.h"
#include "WidgetOpenGL.h"

class WidgetDepthView : public WidgetOpenGL, public SubWindowWidget
{
    private:
        Ptr<DataStream> stream;
        Texture texture;

        FrameNum frameNum;
        DepthFrame frame;

        unsigned char textureBuffer[DepthFrame::SIZE * 4];

    public:
        WidgetDepthView(MainWindow& mainWindow, Ptr<DataStream> stream) :
            WidgetOpenGL(mainWindow),
            stream(stream)
        {
            makeCurrent();
            texture = RenderUtils::createTexture(DepthFrame::WIDTH, DepthFrame::HEIGHT);

            stream->addNewFrameCallback(this, [this](const ColorFrame*, const DepthFrame*, const SkeletonFrame*) -> void {
                emit this->triggerRefresh();
            });
        }

        ~WidgetDepthView()
        {
            if (stream) stream->removeNewFrameCallback(this);
        }

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

        void frameToImage()
        {
            const DepthPixel* src = frame.pixels;
            const DepthPixel* end = src + (DepthFrame::WIDTH * DepthFrame::HEIGHT);

            unsigned char* dest = textureBuffer;

            while (src < end) {
                USHORT depth = src->depth;

                const int MIN_DEPTH = NUI_IMAGE_DEPTH_MINIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
                const int MAX_DEPTH = NUI_IMAGE_DEPTH_MAXIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

                int valid = int(depth >= MIN_DEPTH && depth <= MAX_DEPTH);

                // CLAMP
                //valid = 1;
                /*depth -= ((depth - MAX_DEPTH) & -(depth > MAX_DEPTH));
                depth -= ((depth - MIN_DEPTH) & -(depth < MIN_DEPTH));*/

                BYTE val = (BYTE)(255 - ((depth - MIN_DEPTH) * 256) / (MAX_DEPTH - MIN_DEPTH));

                *dest++ = val | -(1 - valid);
                *dest++ = val & (-valid);
                *dest++ = val & (-valid);
                *dest++ = 0xFF;

                ++src;
            }
        }


        bool render()
        {
            stream->getDepthFrame(frame, &frameNum);
            frameToImage();

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, DepthFrame::WIDTH, DepthFrame::HEIGHT, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();


            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DepthFrame::WIDTH, DepthFrame::HEIGHT,
                GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)textureBuffer);

            RenderUtils::setColor(1.0f, 1.0f, 1.0f, 1.0f);

            RenderUtils::setTexture(texture);
            RenderUtils::drawRect(0.0f, 0.0f, DepthFrame::WIDTH, DepthFrame::HEIGHT);
            RenderUtils::setTexture(0);

            return true;
        }

};

#endif
