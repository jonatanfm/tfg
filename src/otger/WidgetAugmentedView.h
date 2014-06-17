#ifndef WIDGETAUGMENTEDVIEW_H
#define WIDGETAUGMENTEDVIEW_H

#pragma once

#include "../globals.h"

#include "../SubWindow.h"
#include "../WidgetOpenGL.h"

#include "../MainWindow.h"

#include <QGLShaderProgram>

class MainWindow;

// Widget renderer that displays an augmented view, showing a real scene with virtual objects.
class WidgetAugmentedView : public RendererOpenGL, protected QGLFunctions
{
    Q_OBJECT

    private:

        Ptr<DataStream> stream; // The stream that provides color/depth/skeleton data.

        World& world; // The virtual world

        Texture textureColor; // Color image texture
        Texture textureDepth; // Depth data texture

        FrameNum frameNum;
        ColorFrame colorFrame; // The input color frame
        DepthFrame depthFrame; // The input depth frame
        DepthFrame depthBuffer; // Temporary depth buffer (for correction)

        // Output buffer for depth-to-color frame mapping.
        ALIGN(16) NUI_DEPTH_IMAGE_POINT mapping[ColorFrame::SIZE];

        // Buffer for updatig the depth texture contents.
        ALIGN(16) GLushort textureDepthBuffer[ColorFrame::SIZE];

        // Textures and shaders cache
        RenderManager renderManager;

        int depthCorrectionMethod;

        // Shaders
        QGLShaderProgram shaderDefault, shader2D, shaderNormalMap;

    public:
        WidgetAugmentedView(Ptr<DataStream> stream, World& world);

        ~WidgetAugmentedView();

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

        virtual void createActions(QToolBar* toolbar) override;

        void initialize() override;

        bool render() override;


        void nextDepthCorrectionMethod();

    public slots:

        void spawnObject();
        void spawnCube();
        void spawnBallSmall();
        void spawnBallBig();

        void clearObjects();

        void changeDepthCorrectionMethod();
};


#endif
