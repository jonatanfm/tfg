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


        void mousePressEvent(WidgetOpenGL* w, QMouseEvent* ev)
        {
            if (w->is<WidgetDepthView>()) {
                int x = (ev->x() * DepthFrame::WIDTH) / w->width();
                int y = (ev->y() * DepthFrame::HEIGHT) / w->height();

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


        void mouseReleaseEvent(WidgetOpenGL* w, QMouseEvent* ev)
        {
            if (w->is<WidgetDepthView>()) {

            }
        }


        void moveEvent(WidgetOpenGL* w, QMoveEvent* ev)
        {
            if (w->is<WidgetDepthView>()) {

            }
        }
};

#endif
