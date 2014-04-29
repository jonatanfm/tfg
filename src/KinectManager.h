#ifndef KINECTMANAGER_H
#define KINECTMANAGER_H

#pragma once

#include "globals.h"
#include "KinectStream.h"

class KinectManager
{
    friend void CALLBACK deviceStatusCallback(HRESULT, const OLECHAR*, const OLECHAR*, void*);

    public:
        KinectManager();
        ~KinectManager();

        int getSensorCount()
        {
            return kinects.size();
        }

        Ptr<KinectStream> getStream(int i)
        {
            return kinects[i];
        }
    
    private:
        std::vector< Ptr<KinectStream> > kinects;

        void deviceStatusChanged(const OLECHAR* deviceId, bool connected);

};

#endif