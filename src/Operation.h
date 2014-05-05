#ifndef OPERATION_H
#define OPERATION_H

#pragma once

#include "globals.h"

#include "MainWindow.h"
#include "DataStream.h"


class Operation : public QObject
{
    Q_OBJECT

    signals:
        void statusChanged(QString);

        void progressChanged(int value, int max);

    public:
        virtual void run() = 0;
};


class AsyncOperation : public QObject
{
    Q_OBJECT

    public:
        typedef std::function< void() > Callback;


    private:
        QThread thread;
        Operation* worker;

        Callback callback;

        volatile bool running;


    signals:
        void finished();


    private slots:
        void process()
        {
            running = true;

            worker->run();

            running = false;
            emit finished();

            if (callback != nullptr) callback();
        }


    public:
        AsyncOperation(Operation* worker, Callback callback = nullptr) :
            worker(worker),
            callback(callback),
            running(false)
        {
            this->moveToThread(&thread);
            connect(&thread, SIGNAL(started()), this, SLOT(process()));
            connect(this, SIGNAL(finished()), &thread, SLOT(quit()));
            //connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
            //connect(&thread, SIGNAL(finished()), &thread, SLOT(deleteLater()));
        }

        virtual ~AsyncOperation()
        {
            stop();
            if (worker != nullptr) {
                delete worker;
                worker = nullptr;
            }
        }

        void start()
        {
            thread.start();
        }

        void stop()
        {
            running = false;
            if (thread.isRunning()) {
                thread.wait();
            }
        }

        bool isRunning() const
        {
            return thread.isRunning();
        }

        Operation* getOperation() const
        {
            return worker;
        }

};


#endif
