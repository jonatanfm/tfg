
#include "KinectManager.h"

void CALLBACK deviceStatusCallback(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* /*uniqueDeviceName*/, void* pUserData)
{
    ((KinectManager*)pUserData)->deviceStatusChanged(instanceName, SUCCEEDED(hrStatus));
}

KinectManager::KinectManager()
{
    qDebug("Initializing Kinect Manager");

    NuiSetDeviceStatusCallback(&deviceStatusCallback, this);

    int numSensors;
    NuiGetSensorCount(&numSensors);

    if (numSensors < 0) {
        qDebug("ERROR ASKING THE KINECT COUNT!", numSensors);
    }
    else {
        qDebug("Found %d sensor(s)", numSensors);
        if (numSensors > 0) {
            kinects.resize(numSensors);
            for (int i = 0; i < numSensors; ++i) {
                Ptr<KinectStream> k(new KinectStream());
                k->initialize(i);
                kinects.push_back(k);
            }
        }
    }
}

KinectManager::~KinectManager()
{
    for (unsigned int i = 0; i < kinects.size(); ++i) {
        if (kinects[i] != nullptr) kinects[i]->release();
    }
}

void KinectManager::deviceStatusChanged(const OLECHAR* deviceId, bool connected)
{
    if (connected) {
        qDebug() << "Connected Kinect: " << deviceId;
        for (unsigned int i = 0; i < kinects.size(); ++i) {
            if (kinects[i]->sensor != nullptr) {
                OLECHAR* id = kinects[i]->sensor->NuiDeviceConnectionId();
                if (wcscmp(id, deviceId) == 0) {
                    // TODO: clear threads errors
                    return;
                }
            }
        }
        // If not found, add it
        kinects.push_back(Ptr<KinectStream>(new KinectStream()));
        kinects.back()->initializeById(deviceId);
    }
    else { // Disconnected
        qDebug() << "Disconected Kinect: " << deviceId;
        for (unsigned int i = 0; i < kinects.size(); ++i) {
            if (kinects[i]->sensor != nullptr) {
                OLECHAR* id = kinects[i]->sensor->NuiDeviceConnectionId();
                if (wcscmp(id, deviceId) == 0) {
                    kinects[i]->release();
                    kinects.erase(kinects.begin() + i);
                    --i;
                }
            }
        }      
    }
}
