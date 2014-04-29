#ifndef MODE_H
#define MODE_H

#pragma once

#include "globals.h"

class MainWindow;

class Mode
{
    private:
        MainWindow* window;

    public:

        Mode(MainWindow* window) : window(window) { }

        virtual void mousePressEvent(WidgetOpenGL* w, QMouseEvent* ev) { }

        virtual void mouseReleaseEvent(WidgetOpenGL* w, QMouseEvent* ev) { }

        virtual void moveEvent(WidgetOpenGL* w, QMoveEvent* ev) { }
};

#endif
