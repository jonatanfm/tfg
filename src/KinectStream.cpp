#include "KinectStream.h"

#include "Utils.h"


// Coordinate Spaces: http://msdn.microsoft.com/en-us/library/hh973078.aspx

#ifdef HAS_KINECT_TOOLKIT

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
            if (pInteractionInfo) {
                pInteractionInfo->IsPressTarget = false;
                pInteractionInfo->PressTargetControlId = 0;
                pInteractionInfo->PressAttractionPointX = 0.f;
                pInteractionInfo->PressAttractionPointY = 0.f;
                pInteractionInfo->IsGripTarget = true;
            }
            return S_OK;
        }
} interactionClient;

#endif


std::string getErrorString(DWORD errorCode)
{
    LPVOID buffer;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_MAX_WIDTH_MASK,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buffer,
        0, nullptr);

    std::string msg((LPSTR)buffer);

    LocalFree(buffer);

    return msg;
}

void reportError(DWORD errorCode, const char* functionCall)
{
    std::string msg = std::string("Failed call: ") + std::string(functionCall) + std::string("\n\nError: ") + getErrorString(errorCode);
    MessageBoxA(nullptr, msg.c_str(), "Kinect Initialization Error", MB_OK);
}


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
    #ifdef HAS_KINECT_TOOLKIT
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
    if (NuiCreateSensorByIndex(sensorIndex, &sensor) != S_OK) {
        qDebug("Unable to connect to kinect %d\n", sensorIndex);
        return false;
    }
    return initializeStreams();
}

bool KinectStream::initializeById(const OLECHAR* id)
{
    if (NuiCreateSensorById(id, &sensor) != S_OK) {
        qDebug() << "Unable to connect to kinect with id '" << ((QChar*)id) << "'";
        return false;
    }
    return initializeStreams();
}


#define STRINGIFY(x) #x

#define CHECK(funcCall) \
    if (FAILED(res = (funcCall))) { \
        reportError(res, STRINGIFY(funcCall)); \
        return false; \
    }

bool KinectStream::initializeStreams()
{
    HRESULT res;

    // Player Index not supported for 640x480 depth map resolution
    const bool playerIndex = (DepthFrame::WIDTH < 640);

    DWORD flags = NUI_INITIALIZE_FLAG_USES_COLOR | NUI_INITIALIZE_FLAG_USES_SKELETON |
        (playerIndex ? NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX : NUI_INITIALIZE_FLAG_USES_DEPTH);

    CHECK(sensor->NuiInitialize(flags));

    CHECK(sensor->NuiGetCoordinateMapper(&mapper));

    CHECK(sensor->NuiImageStreamOpen(
        NUI_IMAGE_TYPE_COLOR,
        ColorFrame::RESOLUTION,
        0,
        2,
        nullptr,
        &colorStream
    ));

    CHECK(sensor->NuiImageStreamOpen(
        playerIndex ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH,
        DepthFrame::RESOLUTION,
        0, // 0 or NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE (Only Kinect for Windows)
        2,
        nullptr,
        &depthStream
    ));

    CHECK(sensor->NuiSkeletonTrackingEnable(nullptr, 0));

    CHECK(sensor->NuiSetFrameEndEvent(hEvent, 0));

    #ifdef HAS_KINECT_TOOLKIT
        if (playerIndex) {

            CHECK(NuiCreateInteractionStream(sensor, &interactionClient, &interactionStream));

            CHECK(interactionStream->Enable(hInteractionEvent));
        }
    #endif

    //updater = std::thread(runUpdaterWrapper, this);

    start();

    return true;
}

// Assumes size % 4 == 0
void convertBGRA2RGBA(unsigned char* buffer, unsigned int size)
{
    const unsigned char* end = buffer + size;

    #ifndef NOT_VECTORIZED
        // Vectorized assuming size % 16 == 0
        __m128i shuffle = _mm_set_epi8(15, 12, 13, 14, 11, 8, 9, 10, 7, 4, 5, 6, 3, 0, 1, 2);
        for (; buffer < end; buffer += 16) {
            __m128i v = _mm_load_si128((__m128i*) buffer);
            _mm_store_si128((__m128i*) buffer, _mm_shuffle_epi8(v, shuffle));
        }
    #else
        for (; buffer < end; buffer += 4) {
            unsigned char aux = *buffer;
            *buffer = *(buffer + 2);
            *(buffer + 2) = aux;
        }
    #endif
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

    if (skeletonSmoothing) sensor->NuiTransformSmooth(&frame, nullptr);

    skeletonBuffer.frame = frame;
}

void KinectStream::updateInteractions()
{
    #ifdef HAS_KINECT_TOOLKIT
        Vector4 reading = { 0 };
        sensor->NuiAccelerometerGetCurrentReading(&reading);

        HRESULT res;

        res = interactionStream->ProcessDepth(DepthFrame::BYTES, (BYTE*) depthBuffer.pixels, depthTimestamp);
        res = interactionStream->ProcessSkeleton(NUI_SKELETON_COUNT, skeletonBuffer.frame.SkeletonData, &reading, skeletonBuffer.frame.liTimeStamp);
    
        NUI_INTERACTION_FRAME frame;
        res = interactionStream->GetNextFrame(0, &frame);
        if (res == S_OK) {
            for (int i = 0; i < NUI_SKELETON_COUNT; ++i) {
                //if (frame.UserInfos[i].SkeletonTrackingId == 0) continue;
                for (int j = 0; j < NUI_USER_HANDPOINTER_COUNT; ++j) {
                    const char *s1 = "?", *s2 = "?", *s3 = "?";
                    NUI_HANDPOINTER_INFO& ev = frame.UserInfos[i].HandPointerInfos[j];
    
                    //if (ev.HandEventType == NUI_HAND_EVENT_TYPE_NONE) continue;

                    switch (ev.HandEventType)
                    {
                        case NUI_HAND_EVENT_TYPE_NONE: s1 = "EVENT_NONE"; break;
                        case NUI_HAND_EVENT_TYPE_GRIP: s1 = "EVENT_GRIP"; break;
                        case NUI_HAND_EVENT_TYPE_GRIPRELEASE: s1 = "EVENT_GRIPRELEASE"; break;
                    };

                    switch (ev.HandType)
                    {
                        case NUI_HAND_TYPE_NONE: s2 = "HAND_NONE"; break;
                        case NUI_HAND_TYPE_LEFT: s2 = "HAND_LEFT"; break;
                        case NUI_HAND_TYPE_RIGHT: s2 = "HAND_RIGHT"; break;
                    };

                    switch (ev.State)
                    {
                        case NUI_HANDPOINTER_STATE_NOT_TRACKED: s3 = "STATE_NOT_TRACKED"; break;
                        case NUI_HANDPOINTER_STATE_TRACKED: s3 = "STATE_TRACKED"; break;
                        case NUI_HANDPOINTER_STATE_ACTIVE: s3 = "STATE_ACTIVE"; break;
                        case NUI_HANDPOINTER_STATE_INTERACTIVE: s3 = "STATE_INTERACTIVE"; break;
                        case NUI_HANDPOINTER_STATE_PRESSED: s3 = "STATE_PRESSED"; break;
                        case NUI_HANDPOINTER_STATE_PRIMARY_FOR_USER: s3 = "STATE_PRIMARY_FOR_USER"; break;
                    };

                    qDebug() << "Skeleton [" << frame.UserInfos[i].SkeletonTrackingId << "]" << s1 << " " << s2 << " " << s3;
                }
            }
            
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

        #ifdef HAS_KINECT_TOOLKIT
            if (interactionStream != nullptr) updateInteractions();
        #endif

        pushFrame(&colorBuffer, &depthBuffer, &skeletonBuffer);
    }

    if (hr == WAIT_FAILED) {
        qDebug("[KinectStream] Thread exited - wait failed with code %d", GetLastError());
    }
    else qDebug("[KinectStream] Thread exited with %d", hr);
}


bool KinectStream::mapColorFrameToDepthFrame(const DepthFrame& frame, OUT NUI_DEPTH_IMAGE_POINT mapped[ColorFrame::SIZE])
{
    return TRUE == SUCCEEDED(mapper->MapColorFrameToDepthFrame(
        NUI_IMAGE_TYPE_COLOR,
        ColorFrame::RESOLUTION,
        DepthFrame::RESOLUTION,
        DepthFrame::SIZE,
        (NUI_DEPTH_IMAGE_PIXEL*)frame.pixels,
        ColorFrame::SIZE,
        mapped
    ));
}

