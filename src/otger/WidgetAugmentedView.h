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

        FrameNum frameNum;
        ColorFrame frame;

    public:
        WidgetAugmentedView(MainWindow& mainWindow) :
            WidgetOpenGL(mainWindow)
        {
            auto& streams = mainWindow.getStreams();
            if (streams.size() < 1) return;
            
            stream = streams[0];

            makeCurrent();
            texture = RenderUtils::createTexture(ColorFrame::WIDTH, ColorFrame::HEIGHT);

            stream->addNewFrameCallback(this, [this](const ColorFrame*, const DepthFrame*, const SkeletonFrame*) -> void {
                emit this->triggerRefresh();
            });
        }

        ~WidgetAugmentedView()
        {
            if (stream) stream->removeNewFrameCallback(this);
        }

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

        bool render()
        {
            if (!stream) return false;
            stream->getColorFrame(frame, &frameNum);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0, ColorFrame::WIDTH, ColorFrame::HEIGHT, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();


            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ColorFrame::WIDTH, ColorFrame::HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)frame.pixels);

            RenderUtils::setColor(1.0f, 1.0f, 1.0f, 1.0f);

            RenderUtils::setTexture(texture);
            RenderUtils::drawRect(0.0f, 0.0f, ColorFrame::WIDTH, ColorFrame::HEIGHT);
            RenderUtils::setTexture(0);

            World& world = mainWindow.getWorld();

            // TODO

            return true;
        }

};


#endif
