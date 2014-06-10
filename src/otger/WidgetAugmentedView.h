#ifndef WIDGETAUGMENTEDVIEW_H
#define WIDGETAUGMENTEDVIEW_H

#pragma once

#include "../globals.h"

#include "../SubWindow.h"
#include "../WidgetOpenGL.h"

#include "../MainWindow.h"

#include <QGLShaderProgram>

class MainWindow;

// Widget that displays an augmented view, showing a real scene with virtual objects.
class WidgetAugmentedView : public WidgetOpenGL, protected QGLFunctions, public SubWindowWidget
{
    Q_OBJECT

    private:

        Ptr<DataStream> stream; // The stream that provides color/depth/skeleton data.

        Texture textureColor; // Color image texture
        Texture textureDepth; // Depth data texture

        FrameNum frameNum;
        ColorFrame colorFrame; // The input color frame
        DepthFrame depthFrame; // The input depth frame
        DepthFrame depthBuffer; // Temporary depth buffer (for correction)

        // Output buffer for depth-to-color frame mapping.
        __declspec(align(16))
        NUI_DEPTH_IMAGE_POINT mapping[DepthFrame::SIZE];

        // Buffer for updatig the depth texture contents.
        __declspec(align(16))
        GLushort textureDepthBuffer[DepthFrame::SIZE];

        // Textures cache
        TextureManager textures;

        int depthCorrectionMethod;

        // Shaders
        QGLShaderProgram shaderDefault, shader2D;

    public:
        WidgetAugmentedView(MainWindow& mainWindow);

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
        void spawnBall();

        void clearObjects();

        void changeDepthCorrectionMethod();
};


#endif
