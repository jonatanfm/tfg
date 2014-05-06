#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#pragma once

#include <QMdiSubWindow>

#include "WidgetOpenGL.h"

#include "DataStream.h"

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


class SubWindowWidget
{
    public:
        virtual ~SubWindowWidget() { };

        // Get the main stream used in this subwindow, or null if no stream is used
        virtual Ptr<DataStream> getStream() const
        {
            return nullptr;
        }

};


#endif
