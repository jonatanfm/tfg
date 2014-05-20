#ifndef WIDGETCOLORVIEW_H
#define WIDGETCOLORVIEW_H

#pragma once

#include "globals.h"

#include "RenderUtils.h"
#include "DataStream.h"
#include "WidgetOpenGL.h"

class WidgetColorView : public WidgetOpenGL, public SubWindowWidget
{
    private:
        Ptr<DataStream> stream;
        Texture texture;

        FrameNum frameNum;
        ColorFrame frame;

    public:
        WidgetColorView(MainWindow& mainWindow, Ptr<DataStream> stream) :
            WidgetOpenGL(mainWindow),
            stream(stream)
        {
            makeCurrent();
            texture = RenderUtils::createTexture(ColorFrame::WIDTH, ColorFrame::HEIGHT);

            stream->addNewFrameCallback(this, [this](const ColorFrame*, const DepthFrame*, const SkeletonFrame*) -> void {
                emit this->triggerRefresh();
            });
        }

        ~WidgetColorView()
        {
            if (stream) stream->removeNewFrameCallback(this);
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
            glOrtho(0, ColorFrame::WIDTH, ColorFrame::HEIGHT, 0, -1, 1);

            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();


            glBindTexture(GL_TEXTURE_2D, texture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ColorFrame::WIDTH, ColorFrame::HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)frame.pixels);

            RenderUtils::setColor(1.0f, 1.0f, 1.0f, 1.0f);

            RenderUtils::setTexture(texture);
            RenderUtils::drawRect(0.0f, 0.0f, ColorFrame::WIDTH, ColorFrame::HEIGHT);
            RenderUtils::setTexture(0);

            return true;
        }

};

#endif
