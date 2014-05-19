#ifndef WIDGETOPENGL_H
#define WIDGETOPENGL_H

#pragma once

#include "globals.h"

#include <QGLWidget>
#include <QBasicTimer>

#include <functional>

class MainWindow;
class DataStream;

// QT Widget that handles an OpenGL view.
// Inherit from this and override "void initialize()" and "bool render()"
class WidgetOpenGL : public QGLWidget
{
    Q_OBJECT

    public:
        typedef std::function< bool (WidgetOpenGL*) > Overlay;

        enum Type
        {
            TypeColorFrameView,
            DepthFrameView,
            OtherView
        };

        explicit WidgetOpenGL(MainWindow& mainWindow, QWidget *parent = 0);
        ~WidgetOpenGL();

        bool hasOverlay(const std::string& name);

        void addOverlay(const std::string& name, Overlay overlay);

        void removeOverlay(const std::string& name);

        void removeAllOverlays();

        template<class T>
        inline bool is() const
        {
            return dynamic_cast<const T*>(this) != nullptr;
        }

    signals:
        void triggerRefresh();

    protected:

        MainWindow& mainWindow;

        void setFPS(int fps)
        {
            this->fps = fps;
        }

        virtual void initialize()
        {

        }

        virtual bool render()
        {
            return false;
        }

        virtual void onTick()
        {

        }

        void mousePressEvent(QMouseEvent* ev) override;
        void mouseReleaseEvent(QMouseEvent* ev) override;
        void moveEvent(QMoveEvent* ev) override;

    private:

        int fps;

        QBasicTimer timer;

        std::vector< std::pair<std::string, Overlay> > overlays;


        WidgetOpenGL(const WidgetOpenGL&);
        WidgetOpenGL& operator=(const WidgetOpenGL&);

        void initializeGL() override;

        void resizeGL(int w, int h) override;

        void paintGL() override;

        void timerEvent(QTimerEvent *);

};

#endif
