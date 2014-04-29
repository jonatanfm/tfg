
#include "Calibrator.h"

using namespace cv;


#pragma region OPENCV DEBUG

static void collectCalibrationData2(InputArrayOfArrays objectPoints,
    InputArrayOfArrays imagePoints1,
    InputArrayOfArrays imagePoints2,
    Mat& objPtMat, Mat& imgPtMat1, Mat* imgPtMat2,
    Mat& npoints)
{
    int nimages = (int)objectPoints.total();
    int i, j = 0, ni = 0, total = 0;


    int N = (int)imagePoints1.total();
    CV_Assert(nimages > 0);
    CV_Assert(nimages == (int)imagePoints1.total());
    CV_Assert((!imgPtMat2 || nimages == (int)imagePoints2.total()));

    for (i = 0; i < nimages; i++)
    {
        auto& x = objectPoints.getMat(i); // PETA AQUI
        ni = x.checkVector(3, CV_32F);
        CV_Assert(ni >= 0);
        total += ni;
    }

    npoints.create(1, (int)nimages, CV_32S);
    objPtMat.create(1, (int)total, CV_32FC3);
    imgPtMat1.create(1, (int)total, CV_32FC2);
    Point2f* imgPtData2 = 0;

    if (imgPtMat2)
    {
        imgPtMat2->create(1, (int)total, CV_32FC2);
        imgPtData2 = imgPtMat2->ptr<Point2f>();
    }

    Point3f* objPtData = objPtMat.ptr<Point3f>();
    Point2f* imgPtData1 = imgPtMat1.ptr<Point2f>();

    for (i = 0; i < nimages; i++, j += ni)
    {
        Mat objpt = objectPoints.getMat(i);
        Mat imgpt1 = imagePoints1.getMat(i);
        ni = objpt.checkVector(3, CV_32F);
        int ni1 = imgpt1.checkVector(2, CV_32F);
        CV_Assert(ni > 0 && ni == ni1);
        npoints.at<int>(i) = ni;
        memcpy(objPtData + j, objpt.data, ni*sizeof(objPtData[0]));
        memcpy(imgPtData1 + j, imgpt1.data, ni*sizeof(imgPtData1[0]));

        if (imgPtData2)
        {
            Mat imgpt2 = imagePoints2.getMat(i);
            int ni2 = imgpt2.checkVector(2, CV_32F);
            CV_Assert(ni == ni2);
            memcpy(imgPtData2 + j, imgpt2.data, ni*sizeof(imgPtData2[0]));
        }
    }
}

static Mat prepareCameraMatrix(Mat& cameraMatrix0, int rtype)
{
    Mat cameraMatrix = Mat::eye(3, 3, rtype);
    if (cameraMatrix0.size() == cameraMatrix.size())
        cameraMatrix0.convertTo(cameraMatrix, rtype);
    return cameraMatrix;
}

static Mat prepareDistCoeffs(Mat& distCoeffs0, int rtype)
{
    Mat distCoeffs = Mat::zeros(distCoeffs0.cols == 1 ? Size(1, 8) : Size(8, 1), rtype);
    if (distCoeffs0.size() == Size(1, 4) ||
        distCoeffs0.size() == Size(1, 5) ||
        distCoeffs0.size() == Size(1, 8) ||
        distCoeffs0.size() == Size(4, 1) ||
        distCoeffs0.size() == Size(5, 1) ||
        distCoeffs0.size() == Size(8, 1))
    {
        Mat dstCoeffs(distCoeffs, Rect(0, 0, distCoeffs0.cols, distCoeffs0.rows));
        distCoeffs0.convertTo(dstCoeffs, rtype);
    }
    return distCoeffs;
}


double customCalibrateCamera(InputArrayOfArrays _objectPoints,
    InputArrayOfArrays _imagePoints,
    Size imageSize, InputOutputArray _cameraMatrix, InputOutputArray _distCoeffs,
    OutputArrayOfArrays _rvecs, OutputArrayOfArrays _tvecs, int flags, TermCriteria criteria)
{
    int rtype = CV_64F;
    Mat cameraMatrix = _cameraMatrix.getMat();
    cameraMatrix = prepareCameraMatrix(cameraMatrix, rtype);
    Mat distCoeffs = _distCoeffs.getMat();
    distCoeffs = prepareDistCoeffs(distCoeffs, rtype);
    if (!(flags & CALIB_RATIONAL_MODEL))
        distCoeffs = distCoeffs.rows == 1 ? distCoeffs.colRange(0, 5) : distCoeffs.rowRange(0, 5);

    int    i;
    size_t nimages = _objectPoints.total();
    CV_Assert(nimages > 0);
    Mat objPt, imgPt, npoints, rvecM((int)nimages, 3, CV_64FC1), tvecM((int)nimages, 3, CV_64FC1);
    collectCalibrationData2(_objectPoints, _imagePoints, noArray(),
        objPt, imgPt, 0, npoints);
    CvMat c_objPt = objPt, c_imgPt = imgPt, c_npoints = npoints;
    CvMat c_cameraMatrix = cameraMatrix, c_distCoeffs = distCoeffs;
    CvMat c_rvecM = rvecM, c_tvecM = tvecM;

    double reprojErr = cvCalibrateCamera2(&c_objPt, &c_imgPt, &c_npoints, imageSize,
        &c_cameraMatrix, &c_distCoeffs, &c_rvecM,
        &c_tvecM, flags, criteria);

    bool rvecs_needed = _rvecs.needed(), tvecs_needed = _tvecs.needed();

    if (rvecs_needed)
        _rvecs.create((int)nimages, 1, CV_64FC3);
    if (tvecs_needed)
        _tvecs.create((int)nimages, 1, CV_64FC3);

    for (i = 0; i < (int)nimages; i++)
    {
        if (rvecs_needed)
        {
            _rvecs.create(3, 1, CV_64F, i, true);
            Mat rv = _rvecs.getMat(i);
            memcpy(rv.data, rvecM.ptr<double>(i), 3 * sizeof(double));
        }
        if (tvecs_needed)
        {
            _tvecs.create(3, 1, CV_64F, i, true);
            Mat tv = _tvecs.getMat(i);
            memcpy(tv.data, tvecM.ptr<double>(i), 3 * sizeof(double));
        }
    }
    cameraMatrix.copyTo(_cameraMatrix);
    distCoeffs.copyTo(_distCoeffs);

    return reprojErr;
}

#pragma endregion OPENCV DEBUG



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

    //drawChessboardCorners(img, chessboard, corners, found);

    return true;
}

void Calibrator::generateChessboardPoints(OUT cv::vector<cv::Point3f>& points, float squareSize)
{
    for (int i = 0; i < chessboardSize.height; ++i) {
        for (int j = 0; j < chessboardSize.width; ++j) {
            points.push_back(cv::Point3f(squareSize * j, squareSize * i, 0.0f));
        }
    }
}

void Calibrator::streamCalibration()
{
    // Size of the streams
    cv::Size imageSize(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT);

    cv::Mat frame, gray;

    cv::vector< cv::vector<cv::Point2f> > foundPoints;

    // Reference chessboard points
    cv::vector<cv::Point3f> objectPoints;
    generateChessboardPoints(OUT objectPoints);

    // DEBUG
    cv::VideoCapture capture("D:/Lib/opencv/sources/samples/cpp/left%02d.jpg");

    const int targetFrameCount = objectPoints.size();

    // Main loop - grab and process frames
    int frameCount = 0;
    while (frameCount < targetFrameCount) {

        if (frameCount < 9) capture >> frame;
        //*streams[0] >> frame;

        cv::cvtColor(frame, gray, CV_RGB2GRAY);

        if (!findPoints(frame, gray, INOUT foundPoints)) continue;

        ++frameCount;
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
    double rms = customCalibrateCamera(
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
    // Number of streams to calibrate
    const int N = int(streams.size());

    // Reference chessboard points, to use in the calibration
    cv::vector<cv::Point3f> objectPoints;
    generateChessboardPoints(OUT objectPoints);

    // Size of the streams
    cv::Size imageSize(COLOR_FRAME_WIDTH, COLOR_FRAME_HEIGHT);

    // To store the found points, as well as the reference points, for each frame
    cv::vector< cv::vector<cv::Point3f> > chessboardPoints;
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

        /*FixedFrameStream* fis = dynamic_cast<FixedFrameStream*>((DataStream*) stream);
        if (fis != nullptr) {
        fis->setColorImage(img);
        }*/

        ++frameCount;
    }

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
