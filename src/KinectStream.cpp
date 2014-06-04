#include "KinectStream.h"

#include "Utils.h"


// Coordinate Spaces: http://msdn.microsoft.com/en-us/library/hh973078.aspx

#ifdef HAS_KINECT_TOOLIKIT

// Class required by interaction stream - Stub
class InteractionClient : public INuiInteractionClient
{
    public:

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv)
        {
            return S_OK;
        }

        ULONG STDMETHODCALLTYPE AddRef()
        {
            return 2;
        }

        ULONG STDMETHODCALLTYPE Release()
        {
            return 1;
        }

        HRESULT STDMETHODCALLTYPE GetInteractionInfoAtLocation(
            DWORD skeletonTrackingId, NUI_HAND_TYPE handType,
            FLOAT x, FLOAT y,
            _Out_ NUI_INTERACTION_INFO *pInteractionInfo)
        {
            memset(pInteractionInfo, 0, sizeof(NUI_INTERACTION_INFO));
            return S_OK;
        }
} interactionClient;

#endif


KinectStream::KinectStream() :
    sensor(nullptr),
    colorStream(nullptr),
    depthStream(nullptr),
    interactionStream(nullptr),
    depthTimestamp()
{
    hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    hInteractionEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

KinectStream::~KinectStream()
{
    #ifdef HAS_KINECT_TOOLIKIT
        if (interactionStream != nullptr) {
            interactionStream->Disable();
            interactionStream->Release();
            interactionStream = nullptr;
        }
    #endif

    if (sensor != nullptr) {
        //if (updater.joinable()) updater.join();

        if (hEvent != nullptr) ResetEvent(hEvent); // Trigger the event to avoid deadlocks

        colorStream = nullptr;
        depthStream = nullptr;

        sensor->NuiShutdown();
        sensor = nullptr;
    }

    if (hInteractionEvent != nullptr) CloseHandle(hInteractionEvent);
    if (hEvent != nullptr) CloseHandle(hEvent);
}

void KinectStream::stop()
{
    SetEvent(hEvent);
    AsyncStream::stop();
}

bool KinectStream::initialize(int sensorIndex)
{
    if (NuiCreateSensorByIndex(sensorIndex, &sensor) < 0) {
        qDebug("Unable to connect to kinect %d\n", sensorIndex);
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
    HRESULT res;

    res = sensor->NuiInitialize(
        NUI_INITIALIZE_FLAG_USES_COLOR |
        NUI_INITIALIZE_FLAG_USES_DEPTH |
        NUI_INITIALIZE_FLAG_USES_SKELETON
    );
    if (FAILED(res))
    {
        qDebug("Unable to initialize kinect! (Code %d)", res);
        return false;
    }

    if (FAILED(sensor->NuiGetCoordinateMapper(&mapper)))
    {
        qDebug("Unable to get coordinade mapper for kinect!");
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

    #ifdef HAS_KINECT_TOOLIKIT
        if (FAILED(NuiCreateInteractionStream(sensor, &interactionClient, &interactionStream))) {
            qDebug("Failed NuiCreateInteractionStream!");
            return false;
        }

        if (FAILED(interactionStream->Enable(hInteractionEvent))) {
            qDebug("Failed INuiInteractionStream::Enable!");
            return false;
        }
    #endif


    //updater = std::thread(runUpdaterWrapper, this);

    printf("Connected to kinect!\n");

    start();

    return true;
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

void KinectStream::updateColorBuffer()
{
    NUI_IMAGE_FRAME imageFrame;
    if (FAILED(sensor->NuiImageStreamGetNextFrame(colorStream, 0, &imageFrame))) return;

    NUI_LOCKED_RECT lockedRect;
    INuiFrameTexture* frameTex = imageFrame.pFrameTexture;
    frameTex->LockRect(0, &lockedRect, nullptr, 0);
    if (lockedRect.Pitch != 0) {
        const BYTE* src = reinterpret_cast<const BYTE*>(lockedRect.pBits);

        memcpy(colorBuffer.pixels, src, ColorFrame::BYTES);
        convertBGRA2RGBA((unsigned char*) colorBuffer.pixels, ColorFrame::BYTES);
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
            memcpy(depthBuffer.pixels, frame, DepthFrame::BYTES);
            depthTimestamp = imageFrame.liTimeStamp;
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

    

    skeletonBuffer.frame = frame;
}

void KinectStream::updateInteractions()
{
    #ifdef HAS_KINECT_TOOLKIT
        Vector4 reading = { 0 };
        sensor->NuiAccelerometerGetCurrentReading(&reading);

        interactionStream->ProcessDepth(DepthFrame::BYTES, (BYTE*) depthBuffer.pixels, depthTimestamp);
        interactionStream->ProcessSkeleton(NUI_SKELETON_COUNT, skeletonBuffer.frame.SkeletonData, &reading, skeletonBuffer.frame.liTimeStamp);
    
        NUI_INTERACTION_FRAME frame;
        if (S_OK == interactionStream->GetNextFrame(0, &frame)) {

            qDebug("INTERACTION %d %d", frame.UserInfos->HandPointerInfos->HandEventType, frame.UserInfos->HandPointerInfos->HandType);
        }
    #endif
}

void KinectStream::stream()
{
    qDebug("[KinectStream] Thread started");
    HRESULT hr;
    while ((hr = WaitForSingleObject(hEvent, INFINITE)) == WAIT_OBJECT_0) {
        if (stopping) break;
        //qDebug("[KinectStream] Thread update");
        ResetEvent(hEvent);

        updateColorBuffer();
        updateDepthBuffer();
        updateSkeleton();

        #ifdef HAS_KINECT_TOOLIKIT
            updateInteractions();
        #endif

        pushFrame(&colorBuffer, &depthBuffer, &skeletonBuffer);
    }

    if (hr == WAIT_FAILED) {
        qDebug("[KinectStream] Thread exited - wait failed with code %d", GetLastError());
    }
    else qDebug("[KinectStream] Thread exited with %d", hr);
}


bool KinectStream::mapColorFrameToDepthFrame(const DepthFrame& frame, OUT NUI_DEPTH_IMAGE_POINT* mapped)
{
    return TRUE == SUCCEEDED(mapper->MapColorFrameToDepthFrame(
        NUI_IMAGE_TYPE_COLOR,
        NUI_IMAGE_RESOLUTION_640x480,
        NUI_IMAGE_RESOLUTION_640x480,
        DepthFrame::SIZE,
        (NUI_DEPTH_IMAGE_PIXEL*)frame.pixels,
        DepthFrame::SIZE,
        mapped
    ));
}

