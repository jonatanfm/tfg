#ifndef WIDGETAUGMENTEDVIEW_H
#define WIDGETAUGMENTEDVIEW_H

#pragma once

#include "../globals.h"

#include "../SubWindow.h"
#include "../WidgetOpenGL.h"
#include "../RenderUtils.h"

#include "../MainWindow.h"

class WidgetAugmentedView : public WidgetOpenGL, public SubWindowWidget
{
    private:
        Ptr<DataStream> stream;
        Texture texture;

        DataStream::FrameNum frameNum;
        DataStream::ColorPixel* frame;

    public:
        WidgetAugmentedView(MainWindow& mainWindow) :
            WidgetOpenGL(mainWindow)
        {
            auto& streams = mainWindow.getStreams();
            if (streams.size() > 0) stream = streams[0];

            makeCurrent();
            texture = RenderUtils::createTexture(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT);
            frame = DataStream::newColorFrame();

            stream->addNewFrameCallback(this, [this](const DataStream::ColorPixel*, const DataStream::DepthPixel*, const NUI_SKELETON_FRAME*) -> void {
                emit this->triggerRefresh();
            });
        }

        ~WidgetAugmentedView()
        {
            if (stream) stream->removeNewFrameCallback(this);

            delete[] frame;
        }

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

        bool render()
        {
            stream->getColorFrame(frame, &frameNum);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();


            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)frame);

            RenderUtils::setColor(1.0f, 1.0f, 1.0f, 1.0f);

            RenderUtils::setTexture(texture);
            RenderUtils::drawRect(0.0f, 0.0f, COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT);
            RenderUtils::setTexture(0);

            World& world = mainWindow.getWorld();

            // TODO

            return true;
        }

};


#endif
