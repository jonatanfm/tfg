
#include "WidgetAugmentedView.h"

#include "../RenderUtils.h"

#include "../MainWindow.h"

#include "../KinectStream.h"


#include "shaders/shaders.h"

// Kinect fovY
const float fovY = 43.0f;

// Near/Far clipping planes
const float zNear = 0.01f, zFar = 20.0f;


WidgetAugmentedView::WidgetAugmentedView(MainWindow& mainWindow) :
    WidgetOpenGL(mainWindow)
{
    auto& streams = mainWindow.getStreams();
    if (streams.size() < 1) return;
            
    stream = streams[0];
}

WidgetAugmentedView::~WidgetAugmentedView()
{
    if (stream) stream->removeNewFrameCallback(this);
}

void WidgetAugmentedView::initialize()
{
    initializeGLFunctions();

    qDebug() << "OpenGL version:" << QString((const char*) glGetString(GL_VERSION));

    glCheckError();

    textureColor = RenderUtils::createTexture(ColorFrame::WIDTH, ColorFrame::HEIGHT);

    glCheckError();

    textureDepth = RenderUtils::createTexture(DepthFrame::WIDTH, DepthFrame::HEIGHT, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT);

    glCheckError();

    shaderDefault.addShaderFromSourceCode(QGLShader::Vertex, QString(SHADER_DEFAULT_VERTEX));
    shaderDefault.addShaderFromSourceCode(QGLShader::Fragment, QString(SHADER_DEFAULT_FRAGMENT));
    shaderDefault.link();

    glCheckError();

    shader2D.addShaderFromSourceCode(QGLShader::Vertex, QString(SHADER_2D_VERTEX));
    shader2D.addShaderFromSourceCode(QGLShader::Fragment, QString(SHADER_2D_FRAGMENT));
    shader2D.link();

    glCheckError();

    shader2D.bind();
    shader2D.setUniformValue("Color", 0);
    shader2D.setUniformValue("Depth", 1);
    //shader2D.setUniformValue("zNear", static_cast<GLfloat>(zNear));
    //shader2D.setUniformValue("zFar", static_cast<GLfloat>(zFar));
    shader2D.release();

    glCheckError();

    if (stream) stream->addNewFrameCallback(this, [this](const ColorFrame*, const DepthFrame*, const SkeletonFrame*) -> void {
        emit this->triggerRefresh();
    });
}

bool WidgetAugmentedView::render()
{
    if (!stream) return false;
    stream->getColorFrame(colorFrame);
    stream->getDepthFrame(depthFrame);

    // Setup perspective
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovY, float(ColorFrame::WIDTH) / float(ColorFrame::HEIGHT), zNear, zFar);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    glEnable(GL_DEPTH_TEST);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);


    //
    // Draw real world (2D color image)
    //

    glDepthFunc(GL_ALWAYS);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, textureColor);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, ColorFrame::WIDTH, ColorFrame::HEIGHT,
        GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)colorFrame.pixels);

    glActiveTexture(GL_TEXTURE1);

    glBindTexture(GL_TEXTURE_2D, textureDepth);

    KinectStream* kinect = dynamic_cast<KinectStream*>(stream.obj);
    if (kinect != nullptr) {
        kinect->mapColorFrameToDepthFrame(depthFrame, OUT mapping);

        const NUI_DEPTH_IMAGE_POINT* src = mapping;
        GLushort* dest = textureDepthBuffer;
        GLushort* end = textureDepthBuffer + DepthFrame::SIZE;

        #define SRC(i) static_cast<short>(static_cast<unsigned short>((src + i)->depth))

        // Vectorized assuming DepthFrame::SIZE % 8 == 0
        __m128i min = _mm_set1_epi16(static_cast<short>(DepthFrame::MIN_DEPTH));
        __m128i max = _mm_set1_epi16(static_cast<short>(DepthFrame::MAX_DEPTH));
        for (; dest < end; dest += 8, src += 8) {
            __m128i v = _mm_set_epi16(SRC(7), SRC(6), SRC(5), SRC(4), SRC(3), SRC(2), SRC(1), SRC(0));
            v = _mm_max_epu16(min, _mm_min_epu16(max, v));
            _mm_store_si128((__m128i*)dest, v);
        }

        /*for (; dest < end; ++dest, ++src) {
            unsigned short s = SRC(0);
            s = (s > MAX_DEPTH) ? MAX_DEPTH : s;
            s = (s < MIN_DEPTH) ? MIN_DEPTH : s;
            *dest = static_cast<GLushort>(s);
        }*/

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, DepthFrame::WIDTH, DepthFrame::HEIGHT,
            GL_RED_INTEGER, GL_UNSIGNED_SHORT, (GLvoid*)textureDepthBuffer);
    }

    glActiveTexture(GL_TEXTURE0);

    shader2D.bind();

    RenderUtils::drawRect(-1.0f, 1.0f, 2.0f, -2.0f);

    shader2D.release();

    //
    // Draw augmented world
    //

    glDepthFunc(GL_LESS);

    glScalef(1.0f, 1.0f, -1.0f); // Invert Z axis so that +Z is in front of the camera

    //shaderDefault.bind();

    glBegin(GL_TRIANGLE_STRIP);

        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 2.0f);
        glVertex3f(0.5f, -0.5f, 2.0f);
        glVertex3f(0.5f, 0.5f, 3.5f);

    glEnd();
        
    //shaderDefault.release();

    //mainWindow.getWorld().render();

    return true;
}
