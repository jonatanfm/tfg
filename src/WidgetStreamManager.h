#ifndef WIDGETSTREAMMANAGER_H
#define WIDGETSTREAMMANAGER_H

#pragma once

#include <QWidget>
#include <QListView>

#include "DataStream.h"

#include "MainWindow.h"

class WidgetStreamManager : public QWidget
{
    Q_OBJECT;

    private:
        QListView* list;

        MainWindow& mainWindow;

    public:

        WidgetStreamManager(MainWindow& window) :
            mainWindow(window)
        {
            list = new QListView();

            QPushButton* btnClose = new QPushButton("Close");
            QObject::connect(btnClose, SIGNAL(pressed()), this, SLOT(closeStream()));

            QVBoxLayout* layout = new QVBoxLayout();
            layout->addWidget(list);
            layout->addWidget(btnClose);

            setLayout(layout);
        }

        void refresh()
        {
            auto streams = mainWindow.getStreams();

            QStringList streamList;
            for (int i = 0; i < int(streams.size()); ++i) {
                if (streams[i] == nullptr) streamList << "(Closed)";
                else streamList << (QString("#") + QString::number(i) + QString(" - ") + QString::fromStdString(streams[i]->getName()));
            }
            list->setModel(new QStringListModel(streamList));
        }


    public slots:

        void closeStream()
        {
            auto idxs = list->selectionModel()->selectedIndexes();
            if (idxs.size() == 1) {
                mainWindow.closeStream(idxs[0].row());
                refresh();
            }
        }

};


#endif
