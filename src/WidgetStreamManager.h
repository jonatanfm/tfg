#ifndef WIDGETSTREAMMANAGER_H
#define WIDGETSTREAMMANAGER_H

#pragma once

#include <QWidget>
#include <QListWidget>

#include "DataStream.h"

#include "MainWindow.h"

#include "SubWindow.h"

#include "Calibrator.h"

class WidgetStreamManager : public QWidget, public SubWindowWidget
{
    Q_OBJECT

    private:
        QListWidget* list;

        MainWindow& mainWindow;

        inline DataStream* getStreamPtr(QListWidgetItem* item)
        {
            return (DataStream*) ((unsigned long) item->data(Qt::UserRole).toULongLong());
        }

        inline Ptr<DataStream> getStream(QListWidgetItem* item)
        {
            DataStream* ptr = getStreamPtr(item);
            const auto& streams = mainWindow.getStreams();
            for (int i = 0; i < int(streams.size()); ++i) {
                if (streams[i] == ptr) return streams[i];
            }
            return nullptr;
        }

    public:

        WidgetStreamManager(MainWindow& window) :
            mainWindow(window)
        {
            list = new QListWidget();
            list->setSelectionMode(QAbstractItemView::MultiSelection);

            QPushButton* btnCalibrateStream = new QPushButton("Calibrate Stream");
            QObject::connect(btnCalibrateStream, SIGNAL(pressed()), this, SLOT(calibrateStream()));

            QPushButton* btnCalibrateStreams = new QPushButton("Calibrate Streams");
            QObject::connect(btnCalibrateStreams, SIGNAL(pressed()), this, SLOT(calibrateStreams()));


            QWidget* btnsPanel = new QWidget();
            QGridLayout* btns = new QGridLayout(btnsPanel);
            btnsPanel->setLayout(btns);
            
            btns->addWidget(btnCalibrateStream, 0, 0, 1, 1);
            btns->addWidget(btnCalibrateStreams, 0, 1, 1, 1);


            QVBoxLayout* layout = new QVBoxLayout();
            layout->addWidget(list);
            layout->addWidget(btnsPanel);
            setLayout(layout);
        }

        void refresh()
        {
            std::vector<DataStream*> selection;
            {
                const auto& selected = list->selectedItems();
                for (int i = 0; i < selected.size(); ++i) {
                    selection.push_back(getStreamPtr(selected[i]));
                }
            }

            list->clear();

            const auto& streams = mainWindow.getStreams();

            for (int i = 0; i < int(streams.size()); ++i) {
                if (streams[i] == nullptr) continue;

                QListWidgetItem* item = new QListWidgetItem();
                item->setText(QString("#") + QString::number(i) + QString(" - ") + QString::fromStdString(streams[i]->getName()));
                item->setData(Qt::UserRole, (qulonglong) streams[i].obj);
                list->addItem(item);
            }

            for (int i = 0; i < int(selection.size()); ++i) {
                for (int j = 0; j < list->count(); ++j) {
                    if (getStreamPtr(list->item(j)) == selection[i]) {
                        list->setItemSelected(list->item(j), true);
                        break;
                    }
                }
            }
        }


    public slots:

        void calibrateStream()
        {
            auto selected = list->selectedItems();
            if (selected.size() == 1) {
                Calibrator* calibrator = new Calibrator();
                calibrator->addStream(getStream(selected[0]));
                mainWindow.startOperation(calibrator);
                list->clearSelection();
            }
        }

        void calibrateStreams()
        {
            auto selected = list->selectedItems();
            if (selected.size() > 1) {
                Calibrator* calibrator = new Calibrator();
                for (int i = 0; i < selected.size(); ++i) {
                    calibrator->addStream(getStream(selected[i]));

                    for (int j = 0; j < int(mainWindow.getStreams().size()); ++j) {
                        if (getStreamPtr(selected[i]) == mainWindow.getStreams()[j]) {
                            qDebug() << "Selected: " << QString::number(j);
                        }
                    }
                }
                mainWindow.startOperation(calibrator);
                list->clearSelection();
            }
        }



        /*void closeStreams()
        {
            auto idxs = list->selectionModel()->selectedIndexes();
            if (idxs.size() == 1) {
                mainWindow.closeStream(idxs[0].row());
                refresh();
            }
            refresh();
        }*/

};


#endif
