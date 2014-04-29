#ifndef MODEMEASURE_H
#define MODEMEASURE_H

#pragma once

#include "globals.h"

#include "Mode.h"

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
            if (w->getType() == WidgetOpenGL::DepthFrameView) {

            }
        }


        void mouseReleaseEvent(WidgetOpenGL* w, QMouseEvent* ev)
        {
            if (w->getType() == WidgetOpenGL::DepthFrameView) {

            }
        }


        void moveEvent(WidgetOpenGL* w, QMoveEvent* ev)
        {
            if (w->getType() == WidgetOpenGL::DepthFrameView) {

            }
        }
};

#endif
