#include "WidgetOpenGL.h"

#include "RenderUtils.h"

#include "Mode.h"


RendererOpenGL::RendererOpenGL() :
    widget(nullptr),
    active(true),
    aspectWidth(ColorFrame::WIDTH),
    aspectHeight(ColorFrame::HEIGHT),
    fps(0),
    mode(nullptr)
{
}


void RendererOpenGL::start()
{
    assert(widget != nullptr);
    widget->makeCurrent();
    
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    initialize();

    if (widget->isAsync()) connect(this, SIGNAL(triggerRefresh()), this, SLOT(performRender()));
    else connect(this, SIGNAL(triggerRefresh()), widget, SLOT(updateGL()));

    if (fps > 0) timer.start(1000 / fps, this);
}

void RendererOpenGL::stop()
{
    if (widget->isAsync()) {
        QMutexLocker locker(&mutex);
        active = false;
    }
    else {
        timer.stop();
    }
}

void RendererOpenGL::resize(int w, int h)
{
    QMutexLocker locker(&mutex);
    viewportWidth = w;
    viewportHeight = h;
}

void RendererOpenGL::timerEvent(QTimerEvent *event)
{
    QMutexLocker locker(&mutex);
    if (!active) {
        timer.stop();
        QThread::currentThread()->quit();
        return;
    }

    onTick();

    if (widget->isAsync()) performRender();
    else widget->updateGL();
}

void RendererOpenGL::performRender()
{
    if (!widget->isValid()) return;
    if (!widget->updatesEnabled()) return;
    //if (!widget->paintEngine()->isActive()) return;

    widget->context()->makeCurrent();
    
    glColor3f(1.0f, 1.0f, 1.0f);

    {
        int x = 0, y = 0, w = viewportWidth, h = viewportHeight;
        int targetH = (w * aspectHeight) / aspectWidth;
        if (h < targetH) {
            int targetW = (h * aspectWidth) / aspectHeight;
            x = (w - targetW) / 2;
            w = targetW;
        }
        else {
            y = (h - targetH) / 2;
            h = targetH;
        }
        glViewport(x, y, w, h);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (render()) {
        for (unsigned int i = 0; i < overlays.size(); ++i) {
            overlays[i].second(this);
        }
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
        RenderUtils::drawLine(Point2D(-0.95f, -0.95f), Point2D(0.95f, 0.95f));
        RenderUtils::drawLine(Point2D(-0.95f, 0.95f), Point2D(0.95f, -0.95f));
    }

    if (widget->isAsync()) widget->swapBuffers();
}

bool RendererOpenGL::hasOverlay(const std::string& name)
{
    auto it = overlays.begin();
    while (it != overlays.end()) {
        if (it->first == name) return true;
        ++it;
    }
    return false;
}

void RendererOpenGL::addOverlay(const std::string& name, Overlay overlay)
{
    auto it = overlays.begin();
    while (it != overlays.end()) {
        if (it->first == name) return;
        ++it;
    }
    overlays.push_back(std::make_pair(name, overlay));
    emit triggerRefresh();
}

void RendererOpenGL::removeOverlay(const std::string& name)
{
    auto it = overlays.begin();
    while (it != overlays.end()) {
        if (it->first == name) it = overlays.erase(it);
        else ++it;
    }
    emit triggerRefresh();
}

void RendererOpenGL::removeAllOverlays()
{
    overlays.resize(0);
}

