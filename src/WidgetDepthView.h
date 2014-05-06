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
        DataStream::ColorPixel* buffer;

        DataStream::FrameNum frameNum;
        DataStream::DepthPixel* frame;

    public:
        WidgetDepthView(MainWindow& mainWindow, Ptr<DataStream> stream) :
            WidgetOpenGL(mainWindow),
            stream(stream)
        {
            type = DepthFrameView;

            makeCurrent();
            texture = RenderUtils::createTexture(DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT);
            buffer = new DataStream::ColorPixel[DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT];
            frame = DataStream::newDepthFrame();
        }

        ~WidgetDepthView()
        {
            delete[] frame;
            delete[] buffer;
        }

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

        void frameToImage()
        {
            const DataStream::DepthPixel* src = frame;
            const DataStream::DepthPixel* end = src + (DEPTH_FRAME_WIDTH * DEPTH_FRAME_HEIGHT);

            unsigned char* dest = reinterpret_cast<unsigned char*>(buffer);

            while (src < end) {
                USHORT depth = src->depth;

                const int MIN_DEPTH = NUI_IMAGE_DEPTH_MINIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
                const int MAX_DEPTH = NUI_IMAGE_DEPTH_MAXIMUM >> NUI_IMAGE_PLAYER_INDEX_SHIFT;

                int valid = int(depth >= MIN_DEPTH && depth <= MAX_DEPTH);

                // CLAMP
                /*valid = 1;
                depth -= ((depth - MAX_DEPTH) & -(depth > MAX_DEPTH));
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
            glOrtho(0, COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();


            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DEPTH_FRAME_WIDTH, DEPTH_FRAME_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)buffer);

            RenderUtils::setColor(1.0f, 1.0f, 1.0f, 1.0f);

            RenderUtils::setTexture(texture);
            RenderUtils::drawRect(0.0f, 0.0f, COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT);
            RenderUtils::setTexture(0);

            return true;
        }

};

#endif
