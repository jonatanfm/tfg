#ifndef KINECTMANAGER_H
#define KINECTMANAGER_H

#pragma once

#include "globals.h"
#include "KinectStream.h"

// Manages the connected kinects and handles connection/disconnection events.
class KinectManager : public QObject
{
    Q_OBJECT

    // Static callback when a device is connected/disconnected
    friend void CALLBACK deviceStatusCallback(HRESULT, const OLECHAR*, const OLECHAR*, void*);

    public:
        KinectManager();
        ~KinectManager();

        // Gets the number of connected sensors.
        int getSensorCount()
        {
            return kinects.size();
            /*int n = 0;
            for (size_t i = 0; i < kinects.size(); ++i) n += int(kinects[i] != nullptr);
            return n;*/
        }

        // Gets the stream of the corresponding Kinect sensor (May be null)
        Ptr<KinectStream> getStream(int i)
        {
            if (i < 0 || i >= int(kinects.size())) return nullptr;
            return kinects[i];
        }
    
    private:

        // The list of connected kinects, indexed by index.
        // May contain null values (e.g. one has been disconnected)
        std::vector< Ptr<KinectStream> > kinects;

    private slots:

        // Callback when a device is connected/disconnected
        void deviceStatusChanged(QString deviceId, long status);

    signals:

        void deviceStatusChange(QString deviceId, long status);

};

#endif