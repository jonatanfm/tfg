#include "KinectStream.h"

#include "Utils.h"

// Coordinate Spaces: http://msdn.microsoft.com/en-us/library/hh973078.aspx

KinectStream::KinectStream() :
    sensor(nullptr),
    colorStream(nullptr),
    depthStream(nullptr),
    updating(false)
{
    hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

KinectStream::~KinectStream()
{
    if (hEvent != nullptr) CloseHandle(hEvent);
}

bool KinectStream::initialize(int sensorIndex)
{
    if (NuiCreateSensorByIndex(sensorIndex, &sensor) < 0) {
        fprintf(stderr, "Unable to connect to kinect %d\n", sensorIndex);
        return false;
    }
    return initializeStreams();
}

bool KinectStream::initializeById(const OLECHAR* id)
{
    if (NuiCreateSensorById(id, &sensor) < 0) {
        //fprintf(stderr, "Unable to connect to kinect '%s'\n", id);
        return false;
    }
    return initializeStreams();
}

bool KinectStream::initializeStreams()
{
    if (FAILED(sensor->NuiInitialize(
        NUI_INITIALIZE_FLAG_USES_COLOR |
        NUI_INITIALIZE_FLAG_USES_DEPTH |
        NUI_INITIALIZE_FLAG_USES_SKELETON
    )))
    {
        qDebug("Unable to initialize a kinect!");
        return false;
    }

    if (FAILED(sensor->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        0,
        2,
        nullptr,
        &colorStream
    ))) {
        qDebug("Failed NuiImageStreamOpen for color!");
        return false;
    }

    if (FAILED(sensor->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_DEPTH,
        NUI_IMAGE_RESOLUTION_640x480,
        0, // 0 or NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE
        2,
        nullptr,
        &depthStream
    ))) {
        qDebug("Failed NuiImageStreamOpen for depth!");
        return false;
    }

    if (FAILED(sensor->NuiSkeletonTrackingEnable(nullptr, 0))) {
        qDebug("Failed NuiSkeletonTrackingEnable!");
        return false;
    }

    if (FAILED(sensor->NuiSetFrameEndEvent(hEvent, 0))) {
        qDebug("Failed NuiSetFrameEndEvent!");
        return false;
    }

    updating = true;
    this->start();
    //updater = std::thread(runUpdaterWrapper, this);

    printf("Connected to kinect!\n");

    return true;
}


void KinectStream::release()
{
    if (sensor != nullptr) {
        updating = false;
        SetEvent(hEvent);
        if (this->isRunning()) this->wait();
        //if (updater.joinable()) updater.join();

        if (hEvent != nullptr) ResetEvent(hEvent);

        colorStream = nullptr;
        depthStream = nullptr;
    
        sensor->NuiShutdown();
        sensor = nullptr;
    }
}


bool KinectStream::getColorFrame(ColorPixel* data, FrameNum* num)
{
    bool advanced = false;
    colorMutex.lock();
    if (num == nullptr || *num != colorFrameNum) {
        advanced = true;
        if (num != nullptr) *num = colorFrameNum;
        memcpy(data, colorBuffer, COLOR_FRAME_SIZE);
    }
    colorMutex.unlock();
    return advanced;
}

bool KinectStream::getColorImage(cv::Mat& img, FrameNum* num)
{
    bool advanced = false;
    colorMutex.lock();
    if (num == nullptr || *num != colorFrameNum) {
        advanced = true;
        if (num != nullptr) *num = colorFrameNum;
        Utils::colorFrameToRgb((ColorPixel*)colorBuffer, img);
    }
    colorMutex.unlock();
    return advanced;
}


bool KinectStream::getDepthFrame(DepthPixel* data, FrameNum* num)
{
    bool advanced = false;
    depthMutex.lock();
    if (num == nullptr || *num != depthFrameNum) {
        advanced = true;
        if (num != nullptr) *num = depthFrameNum;
        memcpy(data, depthBuffer, DEPTH_FRAME_SIZE);
    }
    depthMutex.unlock();
    return advanced;
}

bool KinectStream::getSkeletonFrame(NUI_SKELETON_FRAME& frame, FrameNum* num)
{
    bool advanced = false;
    skeletonMutex.lock();
    if (skeleton.dwFrameNumber != frame.dwFrameNumber) {
        advanced = true;
        memcpy(&frame, &skeleton, sizeof(NUI_SKELETON_FRAME));
    }
    skeletonMutex.unlock();
    return advanced;
}


void convertBGRA2RGBA(unsigned char* buffer, unsigned int size)
{
    // Assumes size is multiple of 4
    for (unsigned int i = 0; i < size; i += 4) {
        unsigned char aux = buffer[i];
        buffer[i] = buffer[i + 2];
        buffer[i + 2] = aux;
    }
}

bool KinectStream::waitForFrame(ColorPixel* colorFrame, DepthPixel* depthFrame, NUI_SKELETON_FRAME* skeletonFrame, FrameNum* frameNum)
{
    bool signaled;
    newFrameMutex.lock();
    if (signaled = newFrameEvent.wait(&newFrameMutex)) {
        if (colorFrame != nullptr) memcpy(colorFrame, colorBuffer, COLOR_FRAME_SIZE);
        if (depthFrame != nullptr) memcpy(depthFrame, depthBuffer, DEPTH_FRAME_SIZE);
        if (skeletonFrame != nullptr) memcpy(skeletonFrame, &skeleton, sizeof(NUI_SKELETON_FRAME));
    }
    newFrameMutex.unlock();
    return signaled;
}

void KinectStream::updateColorBuffer()
{
    NUI_IMAGE_FRAME imageFrame;
    if (FAILED(sensor->NuiImageStreamGetNextFrame(colorStream, 0, &imageFrame))) return;

    NUI_LOCKED_RECT lockedRect;
    INuiFrameTexture* frameTex = imageFrame.pFrameTexture;
    frameTex->LockRect(0, &lockedRect, nullptr, 0);
    if (lockedRect.Pitch != 0) {
        const BYTE* src = reinterpret_cast<const BYTE*>(lockedRect.pBits);

        colorMutex.lock();
        {
            memcpy(colorBuffer, src, COLOR_FRAME_SIZE);
            convertBGRA2RGBA(colorBuffer, COLOR_FRAME_SIZE);
            ++colorFrameNum;
        }
        colorMutex.unlock();
    }
    frameTex->UnlockRect(0);

    sensor->NuiImageStreamReleaseFrame(colorStream, &imageFrame);
}

void KinectStream::updateDepthBuffer()
{
    NUI_IMAGE_FRAME imageFrame;
    if (FAILED(sensor->NuiImageStreamGetNextFrame(depthStream, 0, &imageFrame))) return;

    NUI_LOCKED_RECT lockedRect;

    BOOL nearMode = FALSE;
    INuiFrameTexture* frameTex;
    HRESULT result = sensor->NuiImageFrameGetDepthImagePixelFrameTexture(depthStream, &imageFrame, &nearMode, &frameTex);
    if (!FAILED(result)) {
        frameTex->LockRect(0, &lockedRect, nullptr, 0);
        if (lockedRect.Pitch != 0) {
            const NUI_DEPTH_IMAGE_PIXEL* frame = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL*>(lockedRect.pBits);

            depthMutex.lock();
            {
                memcpy(depthBuffer, frame, DEPTH_FRAME_SIZE);
                ++depthFrameNum;
            }
            depthMutex.unlock();
        }
        frameTex->UnlockRect(0);
        frameTex->Release();
    }

    sensor->NuiImageStreamReleaseFrame(depthStream, &imageFrame);
}

void KinectStream::updateSkeleton()
{
    NUI_SKELETON_FRAME frame = { 0 };
    HRESULT hr = sensor->NuiSkeletonGetNextFrame(0, &frame);
    if (FAILED(hr)) return; // No data available

    sensor->NuiTransformSmooth(&frame, nullptr);

    skeletonMutex.lock();
    skeleton = frame;
    skeletonMutex.unlock();
}

void KinectStream::run()
{
    qDebug("[KinectStream] Thread started");
    HRESULT hr;
    while ((hr = WaitForSingleObject(hEvent, INFINITE)) == WAIT_OBJECT_0) {
        if (!updating) break;
        //qDebug("[KinectStream] Thread update");
        ResetEvent(hEvent);

        newFrameMutex.lock();

        updateColorBuffer();
        updateDepthBuffer();
        updateSkeleton();
        
        newFrameEvent.wakeAll();
        newFrameMutex.unlock();
    }

    if (hr == WAIT_FAILED) {
        qDebug("[KinectStream] Thread exited - wait failed with code %d", GetLastError());
    }
    else qDebug("[KinectStream] Thread exited with %d", hr);
}
