#ifndef WIDGETOPENGL_H
#define WIDGETOPENGL_H

#pragma once

#include "globals.h"

#include "Data.h"

#include "Mode.h"

#include "SubWindow.h"

#include <QGLWidget>
#include <QBasicTimer>

#include <functional>

class DataStream;


class WidgetOpenGL;

// Handles OpenGL rendering to a WidgetOpenGL.
// May be executed from a non-GUI thread.
class RendererOpenGL : public QObject, public SubWindowContent
{
    Q_OBJECT

    friend class WidgetOpenGL;

    public:

        // Function type for an overlay: a function that draws something over the view.
        typedef std::function< bool(RendererOpenGL*) > Overlay;


        RendererOpenGL();

        virtual ~RendererOpenGL() { }


        // Returns true if contains an overlay with the given name.
        bool hasOverlay(const std::string& name);

        // Adds an overlay with the given name, if it does not already exist.
        void addOverlay(const std::string& name, Overlay overlay);

        // Removes the overlays with the given name.
        void removeOverlay(const std::string& name);

        // Removes all the overlays.
        void removeAllOverlays();

 

        // Stop the renderer (Can be called from other threads)
        void stop();

        // Resize the renderer (Can be called from other threads)
        void resize(int w, int h);


        // Set the pointer to access the current mode
        void setModePointer(const Ptr<Mode>* ptr)
        {
            mode = ptr;
        }

        WidgetOpenGL* getWidget() const
        {
            return widget;
        }

        // Convenience function that returns true if the widget is a instance of the templated classe.
        template<class T>
        inline bool is() const
        {
            return dynamic_cast<const T*>(this) != nullptr;
        }

    public slots:
        // Start the rendering
        void start();

    private slots:
        // Forces to perform a render
        void performRender();

    signals:
        // Force a view refresh
        void triggerRefresh();

    protected:

        // Set the auto-update frequency, or 0 for disabled
        void setFPS(int fps)
        {
            this->fps = fps;
        }

        void setAspect(int w, int h)
        {
            aspectWidth = w;
            aspectHeight = h;
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
        // Warning: they are executed in the GUI thread!

        virtual void mousePressEvent(QMouseEvent* ev) { if (mode != nullptr && mode->obj != nullptr) mode->obj->mousePressEvent(this, ev); }
        virtual void mouseMoveEvent(QMouseEvent* ev) { if (mode != nullptr && mode->obj != nullptr) mode->obj->mouseMoveEvent(this, ev); }
        virtual void mouseReleaseEvent(QMouseEvent* ev) { if (mode != nullptr && mode->obj != nullptr) mode->obj->mouseReleaseEvent(this, ev); }

        virtual void keyPressEvent(QKeyEvent* ev) { }
        virtual void keyReleaseEvent(QKeyEvent* ev) { }

        virtual void focusOutEvent(QFocusEvent* ev) { }


    private:

        // List of registered overlays
        std::vector< std::pair<std::string, Overlay> > overlays;

        QMutex mutex;

        int fps; // The auto-update frequency, or 0 for disabled

        QBasicTimer timer; // Internal timer used if auto-update is enabled.

        WidgetOpenGL* widget;

        int viewportWidth, viewportHeight;

        // Preferred aspect ratio for the widget
        int aspectWidth, aspectHeight;

        bool active;


        // Pointer to the current active mode (if any)
        const Ptr<Mode>* mode;

        // Executed on each timer tick
        void timerEvent(QTimerEvent *event);

        RendererOpenGL(const RendererOpenGL&); // Disable copy constructor
        RendererOpenGL& operator=(const RendererOpenGL&); // Disable assignment

};


// QT Widget that handles an OpenGL view.
// Inherit from this and override "void initialize()" and "bool render()" as needed.
class WidgetOpenGL : public QGLWidget, public SubWindowContent
{
    Q_OBJECT

    friend class WidgetOpenGLPainter;

    public:

        explicit WidgetOpenGL(RendererOpenGL* renderer, QWidget* parent = nullptr) :
            QGLWidget(parent),
            async(false),
            renderer(renderer),
            rendererThread(this)
        {
            assert(renderer != nullptr);
            renderer->widget = this;

            rendererThread.setObjectName("OpenGL Thread");

            setMinimumSize(320, 240);
            //setMaximumSize(640, 480);

            //setAttribute(Qt::WA_PaintOutsidePaintEvent);
            setAttribute(Qt::WA_DeleteOnClose);

            setFocusPolicy(Qt::StrongFocus);

            /*QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
            policy.setHeightForWidth(true);
            setSizePolicy(policy);*/

            if (async) {
                //qDebug("Started thread");
                renderer->moveToThread(&rendererThread);
                connect(&rendererThread, SIGNAL(started()), renderer, SLOT(start()));
                doneCurrent();
                context()->moveToThread(&rendererThread);
                rendererThread.start();
            }
        }

        ~WidgetOpenGL()
        {
            renderer->stop();
            rendererThread.wait();
            delete renderer;
        }


        int heightForWidth(int w) const override
        {
            return (w * renderer->aspectHeight) / renderer->aspectWidth;
        }

        void resizeEvent(QResizeEvent* ev) override
        {
            if (async) renderer->resize(ev->size().width(), ev->size().height());
            else QGLWidget::resizeEvent(ev);
        }

        void paintEvent(QPaintEvent* ev) override
        {
            if (!async) QGLWidget::paintEvent(ev);
            // Otherwise handled by the Painter
        }

        virtual QSize sizeHint() const override
        {
            return QSize(320, 240);
        }

        // Get the renderer object
        inline RendererOpenGL* getRenderer()
        {
            return renderer;
        }


        bool isAsync() const
        {
            return async;
        }


        // SubWindowContent delegated functions

        Ptr<DataStream> getStream() const override { return renderer->getStream(); }

        void createActions(QToolBar* toolbar) override { renderer->createActions(toolbar); }


    protected:

        // Event functions, delegated to the Renderer.

        virtual void mousePressEvent(QMouseEvent* ev) override { renderer->mousePressEvent(ev); }
        virtual void mouseMoveEvent(QMouseEvent* ev) override { renderer->mouseMoveEvent(ev); }
        virtual void mouseReleaseEvent(QMouseEvent* ev) override { renderer->mouseReleaseEvent(ev); }

        virtual void keyPressEvent(QKeyEvent* ev) override { renderer->keyPressEvent(ev); }
        virtual void keyReleaseEvent(QKeyEvent* ev) override { renderer->keyReleaseEvent(ev); }

        virtual void focusOutEvent(QFocusEvent* ev) override { renderer->focusOutEvent(ev); }

    private:

        bool async; // Set to true if uses another thread

        RendererOpenGL* renderer; // The object which handles the rendering

        QThread rendererThread; // The thread running the renderer


        // Called when the OpenGL context has been created
        void initializeGL() override
        {
            if (!async) renderer->start();
        }

        // Called when the view is resized
        void resizeGL(int w, int h) override
        {
            if (!async) renderer->resize(w, h);
        }

        // Called when the view is to be updated
        void paintGL() override
        {
            if (!async) renderer->performRender();
        }

        void glInit() override
        {
            if (!async) QGLWidget::glInit();
        }

        void glDraw() override
        {
            if (!async) QGLWidget::glDraw();
        }

        WidgetOpenGL(const WidgetOpenGL&); // Disable copy constructor
        WidgetOpenGL& operator=(const WidgetOpenGL&); // Disable assignment

};

#endif
