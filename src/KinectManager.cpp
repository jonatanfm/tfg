
#include "KinectManager.h"

void CALLBACK deviceStatusCallback(HRESULT hrStatus, const OLECHAR* instanceName, const OLECHAR* /*uniqueDeviceName*/, void* pUserData)
{
    KinectManager& manager = *((KinectManager*)pUserData);
    emit manager.deviceStatusChange(QString::fromWCharArray(instanceName), static_cast<long>(hrStatus));
}

KinectManager::KinectManager()
{
    qDebug("Initializing Kinect Manager");

    NuiSetDeviceStatusCallback(&deviceStatusCallback, this);

    int numSensors;
    if (NuiGetSensorCount(&numSensors) != S_OK || numSensors < 0) {
        qDebug("Error fetching the Kinect count!", numSensors);
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

    QObject::connect(this, SIGNAL(deviceStatusChange(QString, long)), this, SLOT(deviceStatusChanged(QString, long)));
}

KinectManager::~KinectManager()
{
    NuiSetDeviceStatusCallback(nullptr, nullptr);
}

void KinectManager::deviceStatusChanged(QString deviceId, long status)
{
    HRESULT hrStatus = static_cast<HRESULT>(status);
    if (hrStatus == S_OK) { // Connected
        qDebug() << "Connected Kinect: " << deviceId;
        size_t emptyIndex = std::string::npos;
        for (size_t i = 0; i < kinects.size(); ++i) {
            if (kinects[i] == nullptr) {
                if (emptyIndex == std::string::npos) emptyIndex = i;
            }
            else if (kinects[i]->sensor != nullptr) {
                QString id = QString::fromWCharArray(kinects[i]->sensor->NuiDeviceConnectionId());
                if (deviceId == id) return;
            }
        }
        // If not found, add it
        if (emptyIndex == std::string::npos) kinects.push_back(nullptr);
        kinects[emptyIndex] = Ptr<KinectStream>(new KinectStream());
        kinects[emptyIndex]->initializeById((const OLECHAR*)deviceId.constData());
    }
    else if (hrStatus == E_NUI_NOTCONNECTED) { // Disconnected
        qDebug() << "Disconnected Kinect: " << deviceId;
        for (size_t i = 0; i < kinects.size(); ++i) {
            if (kinects[i] != nullptr && kinects[i]->sensor != nullptr) {
                QString id = QString::fromWCharArray(kinects[i]->sensor->NuiDeviceConnectionId());
                if (deviceId == id) {
                    kinects[i] = nullptr;
                    kinects.erase(kinects.begin() + i);
                    --i;
                }
            }
        }      
    }
    //else if (hrStatus == S_NUI_INITIALIZING) {} // Initializing
}
