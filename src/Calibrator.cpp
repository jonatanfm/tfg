
#include "Calibrator.h"

#include "Utils.h"

#include "FixedFrameStream.h"


bool Calibrator::findPoints(cv::Mat& image, cv::Mat& gray, INOUT cv::vector< cv::vector<cv::Point2f> >& points)
{
    cv::vector<cv::Point2f> corners;

    const int flags =
        CV_CALIB_CB_ADAPTIVE_THRESH | // Use adaptive thresholding to convert the image to black and white, rather than a fixed threshold level(computed from the average image brightness).
        CV_CALIB_CB_FAST_CHECK | // Normalize the image gamma with equalizeHist() before applying fixed or adaptive thresholding.
        CV_CALIB_CB_NORMALIZE_IMAGE | // Use additional criteria (like contour area, perimeter, square-like shape) to filter out false quads extracted at the contour retrieval stage.
        0;

    bool found = findChessboardCorners(
        image,
        chessboardSize,
        OUT corners,
        flags
    );

    if (!found) {
        qDebug("Corners not found!");
        return false;
    }

    //if (CHESSBOARD)
    {
        cornerSubPix(
            gray,
            INOUT corners,
            cv::Size(11, 11),
            cv::Size(-1, -1),
            cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1)
        );
    }

    points.push_back(corners);

    return true;
}

void Calibrator::generateChessboardPoints(OUT cv::Mat& points, float squareSize)
{
    points.create(cv::Size(3, chessboardSize.width * chessboardSize.height), CV_32F);
    for (int i = 0; i < chessboardSize.height; ++i) {
        for (int j = 0; j < chessboardSize.width; ++j) {
            int idx = i * chessboardSize.width + j;
            points.at<float>(idx, 0) = squareSize * j;
            points.at<float>(idx, 1) = squareSize * i;
            points.at<float>(idx, 2) = 0.0f;
        }
    }
    //int ni = cv::InputArray(points).getMat(0).checkVector(3, CV_32F);
}

/*void Calibrator::generateChessboardPointsInVector(OUT cv::vector<cv::Point3f>& points, float squareSize)
{
    for (int i = 0; i < chessboardSize.height; ++i) {
        for (int j = 0; j < chessboardSize.width; ++j) {
            points.push_back(cv::Point3f(squareSize * j, squareSize * i, 0.0f));
        }
    }
    int ni = cv::InputArray(points).getMat(0).checkVector(3, CV_32F);
}*/



void Calibrator::streamCalibration()
{
    // Size of the streams
    cv::Size imageSize(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT);

    cv::Mat frame, gray;

    cv::vector< cv::vector<cv::Point2f> > foundPoints;

    // Reference chessboard points
    //cv::vector<cv::Point3f> objectPoints;
    cv::Mat objectPoints;
    generateChessboardPoints(OUT objectPoints);

    // DEBUG
    cv::VideoCapture capture("D:/Lib/opencv/sources/samples/cpp/left%02d.jpg");

    const int targetFrameCount = objectPoints.rows;
    //const int targetFrameCount = objectPoints.size();

    // Main loop - grab and process frames
    int frameCount = 0;
    while (frameCount < targetFrameCount) {

        if (frameCount < 9) capture >> frame;
        //*streams[0] >> frame;

        cv::cvtColor(frame, gray, CV_RGB2GRAY);

        if (!findPoints(frame, gray, INOUT foundPoints)) continue;

        ++frameCount;
        progressChanged(frameCount, targetFrameCount);
    }

    // Termination criteria for the iterative optimization algorithm
    const cv::TermCriteria termCriteria =
        cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, DBL_EPSILON);

    const int flags = CV_CALIB_FIX_K4 | CV_CALIB_FIX_K5 /*| CV_CALIB_FIX_K6*/;

    cv::vector<cv::Mat> rvecs, tvecs;

    intrinsic.cameraMatrix = cv::Mat::zeros(3, 3, CV_32F);
    intrinsic.cameraMatrix.at<float>(0, 0) = 1;
    intrinsic.cameraMatrix.at<float>(1, 1) = 1;

    //intrinsic.distCoeffs = cv::Mat::zeros(3, 3, CV_64F);


    // WARNING! objectPoints.size() MUST BE EQUAL TO foundPoints.size()!!

    // http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html#calibratecamera
    double rms = calibrateCamera(
        objectPoints,
        foundPoints,
        imageSize,
        INOUT intrinsic.cameraMatrix,
        INOUT intrinsic.distCoeffs,
        OUT rvecs,
        OUT tvecs,
        flags,
        termCriteria
    );

    intrinsic.reprojectionError = rms;
}

void Calibrator::systemCalibration()
{
    statusChanged("Starting callibration...");

    // Number of streams to calibrate
    const int N = int(streams.size());

    // Reference chessboard points, to use in the calibration
    //cv::vector<cv::Point3f> objectPoints;
    cv::Mat objectPoints;
    generateChessboardPoints(OUT objectPoints);

    // Size of the streams
    cv::Size imageSize(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT);

    // To store the found points, as well as the reference points, for each frame
    cv::vector< cv::Mat > chessboardPoints;
    cv::vector< cv::vector< cv::vector<cv::Point2f> > > foundPoints;
    foundPoints.resize(N);

    // Temp vector to hold the grabbed frames
    cv::vector<cv::Mat> frames;
    for (int i = 0; i < N; ++i) {
        frames.push_back(cv::Mat(imageSize, CV_8UC3));
    }

    // Temporal matrix to store a frame converted to gray
    cv::Mat gray;

    // Maximum number of frames to run, unless stopped before
    const int MAX_NUM_FRAMES = 10;

    statusChanged("Grabbing frames...");

    // Main loop - grab and process frames
    int frameCount = 0;
    while (frameCount < MAX_NUM_FRAMES) {

        // Grab one frame from each stream
        for (int i = 0; i < N; ++i) {
            *streams[i] >> frames[i];
        }

        // Convert each frame to gray, and find the points
        int i;
        for (i = 0; i < N; ++i) {
            cv::cvtColor(frames[i], gray, CV_RGB2GRAY);
            if (!findPoints(frames[i], gray, INOUT foundPoints[i])) break;
        }

        // If one frame failed, remove the points dectedted in this frame and skip it
        if (i < N) {
            while (--i >= 0) {
                foundPoints[i].pop_back();
            }
            continue;
        }

        // Add a set of reference points to the vector
        chessboardPoints.push_back(objectPoints);

        ++frameCount;
        progressChanged(frameCount, MAX_NUM_FRAMES);

        #pragma message("TODO: REMOVE THIS DEBUG")
        if (frameCount == MAX_NUM_FRAMES) {
            for (i = 0; i < N; ++i) {
                FixedFrameStream* fis = dynamic_cast<FixedFrameStream*>((DataStream*)streams[i]);
                if (fis != nullptr) {
                    cv::Mat debugImg(frames[i]);
                    drawChessboardCorners(debugImg, chessboardSize, foundPoints[i].back(), true);
                    fis->setColorImage(debugImg);
                }
            }
        }
    }

    statusChanged("Computing parameters...");
    progressChanged(0, 0);

    // Reset the calibration parameters container
    params.resize(N - 1);

    cv::vector<cv::Mat> cameraMatrices;
    for (int i = 0; i < N; ++i) {
        cameraMatrices.push_back(cv::Mat::eye(3, 3, CV_64FC1));
    }

    cv::Mat distCoeffs0, distCoeffs1;

    // Termination criteria for the iterative optimization algorithm
    const cv::TermCriteria termCriteria =
        cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 1e-6);
    //cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 100, 1e-5);

    const int flags =
        //CV_CALIB_FIX_INTRINSIC + // Fix cameraMatrix? and distCoeffs? so that only R, T, E, and F matrices are estimated.
        CV_CALIB_FIX_ASPECT_RATIO + // Fix the ratio f_x/f_y, optimize f_z
        CV_CALIB_ZERO_TANGENT_DIST + // Set tangential distortion coefficients for each camera to zeros and fix there.
        CV_CALIB_SAME_FOCAL_LENGTH + // Enforce same focal length f_x and f_y for both devices
        0;

    // Calibrate the stream 0 with each other stream
    const int i = 0;
    for (int j = 1; j < N; ++j) {

        ExtrinsicParams& params = this->params[j - 1];

        // http://docs.opencv.org/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html#stereocalibrate
        cv::stereoCalibrate(
            chessboardPoints, // Vector of vectors of the calibration pattern points
            foundPoints[i], // Vector of vectors of the projections of the calibration pattern points, observed by the first camera
            foundPoints[j], // Vector of vectors of the projections of the calibration pattern points, observed by the second camera
            INOUT cameraMatrices[i],
            INOUT distCoeffs0,
            INOUT cameraMatrices[j],
            INOUT distCoeffs1,
            imageSize,
            OUT params.R,
            OUT params.T,
            OUT params.E,
            OUT params.F,
            termCriteria,
            flags
        );
    }

}
