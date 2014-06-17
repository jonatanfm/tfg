#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#pragma once

#include <QMdiSubWindow>

#include "DataStream.h"

class MainWindow;

#ifdef _WIN32
    extern QPixmap qt_pixmapFromWinHICON(HICON icon);
#endif

// SubWindow of the MainWindow's Modal Document Interface (MDI) area.
class SubWindow : public QMdiSubWindow
{
    Q_OBJECT

    public:

        explicit SubWindow(MainWindow* window, QWidget* parent = 0) :
            QMdiSubWindow(parent),
            window(window)
        {
            #ifdef _WIN32
                // Set the icon to the app icon.
                HICON hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(102));
                setWindowIcon(QIcon(qt_pixmapFromWinHICON(hIcon)));
            #endif
        }

        ~SubWindow() { }

    protected:
        MainWindow* window;

};


//  that is the root widget of a SubWindow.
class SubWindowContent
{
    public:
        virtual ~SubWindowContent() { };

        // Get the main stream used in this subwindow, or null if no stream is used
        virtual Ptr<DataStream> getStream() const
        {
            return nullptr;
        }

        // Called to add widget-specific actions to the passed toolbar
        virtual void createActions(QToolBar* toolbar)
        {

        }

};


// A simple widget used as a stub view for non-representable streams
class EmptyView : public QLabel, public SubWindowContent
{
    private:
        Ptr<DataStream> stream;

    public:
        EmptyView(MainWindow& mainWindow, Ptr<DataStream> stream) :
            QLabel(),
            stream(stream)
        {
            setText(QString::fromStdString(stream->getName()));
        }

        ~EmptyView()
        {

        }

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

};

#endif
