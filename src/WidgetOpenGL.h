#ifndef WIDGETOPENGL_H
#define WIDGETOPENGL_H

#pragma once

#include "globals.h"

#include "Data.h"

#include <QGLWidget>
#include <QBasicTimer>

#include <functional>

class Mode;
class DataStream;

// QT Widget that handles an OpenGL view.
// Inherit from this and override "void initialize()" and "bool render()" as needed.
class WidgetOpenGL : public QGLWidget
{
    Q_OBJECT

    public:
        // Function type for an overlay: a function that draws something over the view.
        typedef std::function< bool (WidgetOpenGL*) > Overlay;

        explicit WidgetOpenGL(QWidget *parent = 0);
        ~WidgetOpenGL();

        // Returns true if contains an overlay with the given name.
        bool hasOverlay(const std::string& name);

        // Adds an overlay with the given name, if it does not already exist.
        void addOverlay(const std::string& name, Overlay overlay);

        // Removes the overlays with the given name.
        void removeOverlay(const std::string& name);

        // Removes all the overlays.
        void removeAllOverlays();

        // Set the pointer to access the current mode
        void setModePointer(const Ptr<Mode>* ptr)
        {
            mode = ptr;
        }

        int heightForWidth(int w) const override
        {
            return (w * preferredHeight) / preferredWidth;
        }

        // Convenience function that returns true if the widget is a instance of the templated classe.
        template<class T>
        inline bool is() const
        {
            return dynamic_cast<const T*>(this) != nullptr;
        }

    signals:
        // Force a view refresh
        void triggerRefresh();

    protected:

        // Pointer to the current active mode (if any)
        const Ptr<Mode>* mode;

        // Preferred aspect ratio for the widget
        int preferredWidth, preferredHeight;

        // Set the auto-update frequency, or 0 for disabled
        void setFPS(int fps)
        {
            this->fps = fps;
        }

        // Initialize the OpenGL objects
        virtual void initialize()
        {

        }

        // Render the view contents
        virtual bool render()
        {
            return false;
        }

        // Called on each tick of the interal timer
        virtual void onTick()
        {

        }

        // Event functions, delegated to the current interaction Mode by default.

        void mousePressEvent(QMouseEvent* ev) override;
        void mouseReleaseEvent(QMouseEvent* ev) override;
        void moveEvent(QMoveEvent* ev) override;

    private:

        int fps; // The auto-update frequency, or 0 for disabled

        QBasicTimer timer; // Internal timer used if auto-update is enabled.

        // List of registered overlays
        std::vector< std::pair<std::string, Overlay> > overlays;

        // Called when the OpenGL context has been created
        void initializeGL() override;

        // Called when the view is resized
        void resizeGL(int w, int h) override;

        // Called when the view is to be updated
        void paintGL() override;

        // Executed on each timer tick
        void timerEvent(QTimerEvent *);


        WidgetOpenGL(const WidgetOpenGL&); // Disable copy constructor
        WidgetOpenGL& operator=(const WidgetOpenGL&); // Disable assignment


};

#endif
