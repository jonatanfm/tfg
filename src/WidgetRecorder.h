#ifndef WIDGETRECORDER_H
#define WIDGETRECORDER_H

#pragma once

#include <QWidget>
#include <QThread>

#include "DataStream.h"

#include "SubWindow.h"

namespace Ui {
    class WidgetRecorder;
}

class MainWindow;

class Recorder;

class WidgetRecorder : public QWidget, public SubWindowWidget
{
    Q_OBJECT

    public:
        explicit WidgetRecorder(MainWindow& mainWindow, QWidget* parent = 0);
        ~WidgetRecorder();

    private:
        Ui::WidgetRecorder *ui;
        MainWindow& mainWindow;
        
        bool recording;
        std::vector<Recorder*> recorders;

        void updateStreamList();

        void captureColorFrame(Ptr<DataStream> stream, QString filename);
        void captureDepthFrame(Ptr<DataStream> stream, QString filename);
        void captureSkeletonFrame(Ptr<DataStream> stream, QString filename);

    public slots:
        void record();
        void captureFrame();

};

#endif
