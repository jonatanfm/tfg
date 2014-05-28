#ifndef OPERATION_H
#define OPERATION_H

#pragma once

#include "globals.h"

#include "MainWindow.h"
#include "DataStream.h"


// Represents a process that can be executed, and can report its progress.
class Operation : public QObject
{
    Q_OBJECT

    signals:
        // Can be emitted to change the displayed status string.
        void statusChanged(QString);

        // Can be emitted to display a given progress value out of the given maximum.
        void progressChanged(int value, int max);

    public:
        // Reimplemented in child classes to execute the work.
        virtual void run() = 0;
};


// Handles execution of a Operation asynchronously in another thread.
class AsyncOperation : public QObject
{
    Q_OBJECT

    public:
        // Operation finished callback function type
        typedef std::function< void() > Callback;


    private:
        QThread thread; // Thread that executes the operation

        Operation* operation; // The operation to execute

        Callback callback; // Function called when the operation finished (or null)

        volatile bool running; // True if operation is running.


    signals:
        // Called when the thread running the operation finishes.
        void finished();


    private slots:
        // Main function for the executor thread.
        void process()
        {
            running = true;

            operation->run();

            running = false;
            emit finished();

            if (callback != nullptr) callback();
        }


    public:
        AsyncOperation(Operation* op, Callback callback = nullptr) :
            operation(op),
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
            if (operation != nullptr) {
                delete operation;
                operation = nullptr;
            }
        }

        // Start executing the operation asynchronously.
        void start()
        {
            thread.start();
        }

        // Abort the operation.
        void stop()
        {
            running = false;
            if (thread.isRunning()) {
                thread.wait();
            }
        }

        // Returns true if the operation is still running.
        bool isRunning() const
        {
            return thread.isRunning();
        }

        // Returns the operation being executed.
        Operation* getOperation() const
        {
            return operation;
        }

};


#endif
