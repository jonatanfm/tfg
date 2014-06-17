#ifndef MODE_H
#define MODE_H

#pragma once

#include "globals.h"

class MainWindow;

class RendererOpenGL;

// A mode of interaction with the subwindow widgets.
// Handles what happens when a widget receives events.
class Mode
{
    protected:
        MainWindow* window;

    public:

        Mode(MainWindow* window) : window(window) { }

        virtual void mousePressEvent(RendererOpenGL* w, QMouseEvent* ev) { }

        virtual void mouseReleaseEvent(RendererOpenGL* w, QMouseEvent* ev) { }

        virtual void mouseMoveEvent(RendererOpenGL* w, QMouseEvent* ev) { }
};

#endif
