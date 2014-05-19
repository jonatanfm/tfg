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
            if (w->is<WidgetDepthView>()) {

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
