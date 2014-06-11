#include "WidgetOpenGL.h"

#include "RenderUtils.h"

#include "Mode.h"

WidgetOpenGL::WidgetOpenGL(QWidget *parent) :
    QGLWidget(parent),
    mode(nullptr),
    preferredWidth(ColorFrame::WIDTH),
    preferredHeight(ColorFrame::HEIGHT),
    fps(0)
{
    setMinimumSize(320, 240);
    //setMaximumSize(640, 480);

    /*QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);*/

    connect(this, SIGNAL(triggerRefresh()), this, SLOT(updateGL()));
}

WidgetOpenGL::~WidgetOpenGL()
{
    timer.stop();
}

void WidgetOpenGL::initializeGL()
{
    //makeCurrent();

    /*auto f = format();
    qDebug() << f;*/

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);

    initialize();

    if (fps > 0) timer.start(1000 / fps, this);
}

void WidgetOpenGL::resizeGL(int w, int h)
{
    int x = 0, y = 0;
    //makeCurrent();
    int targetH = (w * preferredHeight) / preferredWidth;
    if (h < targetH) {
        int targetW = (h * preferredWidth) / preferredHeight;
        x = (w - targetW) / 2;
        w = targetW;
    }
    else {
        y = (h - targetH) / 2;
        h = targetH;
    }
    glViewport(x, y, w, h);
}

void WidgetOpenGL::paintGL()
{
    //makeCurrent();
    glColor3f(1.0f, 1.0f, 1.0f);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (render()) {
        for (unsigned int i = 0; i < overlays.size(); ++i) {
            overlays[i].second(this);
        }
    }
    else {
        glColor3f(1.0f, 0.0f, 0.0f);
        RenderUtils::drawLine(Point2D(-0.95f, -0.95f), Point2D(0.95f,  0.95f));
        RenderUtils::drawLine(Point2D(-0.95f,  0.95f), Point2D(0.95f, -0.95f));
    }

    //swapBuffers();
}

void WidgetOpenGL::timerEvent(QTimerEvent*)
{
    onTick();
    updateGL();
}

void WidgetOpenGL::mousePressEvent(QMouseEvent* ev)
{
    if (mode != nullptr && mode->obj != nullptr) mode->obj->mousePressEvent(this, ev);
}

void WidgetOpenGL::mouseReleaseEvent(QMouseEvent* ev)
{

    if (mode != nullptr && mode->obj != nullptr) mode->obj->mouseReleaseEvent(this, ev);
}

void WidgetOpenGL::moveEvent(QMoveEvent* ev)
{
    if (mode != nullptr && mode->obj != nullptr) mode->obj->moveEvent(this, ev);
}



bool WidgetOpenGL::hasOverlay(const std::string& name)
{
    auto it = overlays.begin();
    while (it != overlays.end()) {
        if (it->first == name) return true;
        ++it;
    }
    return false;
}

void WidgetOpenGL::addOverlay(const std::string& name, Overlay overlay)
{
    auto it = overlays.begin();
    while (it != overlays.end()) {
        if (it->first == name) return;
        ++it;
    }
    overlays.push_back(std::make_pair(name, overlay));
    updateGL();
}

void WidgetOpenGL::removeOverlay(const std::string& name)
{
    auto it = overlays.begin();
    while (it != overlays.end()) {
        if (it->first == name) it = overlays.erase(it);
        else ++it;
    }
    updateGL();
}

void WidgetOpenGL::removeAllOverlays()
{
    overlays.resize(0);
}