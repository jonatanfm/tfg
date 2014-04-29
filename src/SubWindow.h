#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#pragma once

#include <QMdiSubWindow>

#include "WidgetOpenGL.h"

class MainWindow;

#ifdef _WIN32
    extern QPixmap qt_pixmapFromWinHICON(HICON icon);
#endif

class SubWindow : public QMdiSubWindow
{
    Q_OBJECT

    public:

        explicit SubWindow(MainWindow* window, QWidget* parent = 0) :
            QMdiSubWindow(parent),
            window(window)
        {
            #ifdef _WIN32
                HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(102));
                setWindowIcon(QIcon(qt_pixmapFromWinHICON(hIcon)));
            #endif
        }

        ~SubWindow() { }

    protected:
        MainWindow* window;

};


#endif
