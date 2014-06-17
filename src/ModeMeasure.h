#ifndef MODEMEASURE_H
#define MODEMEASURE_H

#pragma once

#include "globals.h"

#include "Mode.h"

// Mode of interaction that reports distances in the depth view.
// (Not implemented yet)
class MeasureMode : public Mode
{
    private:
        int count;

    public:
        
        MeasureMode(MainWindow* window) :
            Mode(window),
            count(0)
        {

        }


        void mousePressEvent(RendererOpenGL* w, QMouseEvent* ev) override
        {
            if (w->is<WidgetDepthView>()) {
                int x = (ev->x() * DepthFrame::WIDTH) / w->getWidget()->width();
                int y = (ev->y() * DepthFrame::HEIGHT) / w->getWidget()->height();

                x = (x < 0) ? 0 : x;
                x = (x >= DepthFrame::WIDTH) ? DepthFrame::WIDTH - 1 : x;

                y = (y < 0) ? 0 : y;
                y = (y >= DepthFrame::HEIGHT) ? DepthFrame::HEIGHT - 1 : y;

                DepthFrame* frame = new DepthFrame();
                ((WidgetDepthView*)w)->getStream()->getDepthFrame(*frame);
                unsigned short depth = (*frame)(x, y).depth;
                delete frame;

                window->setStatusText(QString("Distance: ") + QString::number(depth / 1000.0f) + " m");
            }
        }

};

#endif
