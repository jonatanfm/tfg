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
    private:

        Ptr<DataStream> stream; // The stream that provides color/depth/skeleton data.

        Texture textureColor; // Color image texture
        Texture textureDepth; // Depth data texture

        FrameNum frameNum;
        ColorFrame colorFrame; // The input color frame
        DepthFrame depthFrame; // The input depth frame

        // Output buffer for depth-to-color frame mapping.
        __declspec(align(16))
        NUI_DEPTH_IMAGE_POINT mapping[DepthFrame::SIZE];

        // Buffer for updatig the depth texture contents.
        __declspec(align(16))
        GLushort textureDepthBuffer[DepthFrame::SIZE];

        // Shaders
        QGLShaderProgram shaderDefault, shader2D;

    public:
        WidgetAugmentedView(MainWindow& mainWindow);

        ~WidgetAugmentedView();

        Ptr<DataStream> getStream() const override
        {
            return stream;
        }

        void initialize() override;

        bool render() override;

};


#endif
