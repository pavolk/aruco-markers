/*
 * Copyright (c) 2019 Flight Dynamics and Control Lab
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <cstdlib>


namespace {
const char* about = "Pose estimation of ArUco marker images";
const char* keys  =
        "{d        |16    | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, "
        "DICT_4X4_250=2, DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, "
        "DICT_5X5_250=6, DICT_5X5_1000=7, DICT_6X6_50=8, DICT_6X6_100=9, "
        "DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12, DICT_7X7_100=13, "
        "DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{h        |false | Print help }"
        "{l        |      | Actual marker length in meter }"
        "{v        |<none>| Custom video source, otherwise '0' }"
        "{h        |false | Print help }"
        "{l        |      | Actual marker length in meter }"
        "{v        |<none>| Custom video source, otherwise '0' }"
        ;
}



typedef std::vector<cv::Point3f> Corners;

std::vector<Corners> createObjectPoints(int markersX, int markersY, float markerLength, float markerSeparationX, float markerSeparationY)
{
    std::vector<Corners> objectPoints;

    // calculate Board objPoints
    float maxY = (float)markersY * markerLength + (markersY - 1) * markerSeparationY;
    for (int y = 0; y < markersY; y++) {
        for (int x = 0; x < markersX; x++) {
            std::vector<cv::Point3f> corners;
            corners.resize(4);
            corners[0] = cv::Point3f(x * (markerLength + markerSeparationX),
                maxY - y * (markerLength + markerSeparationY), 0);
            corners[1] = corners[0] + cv::Point3f(markerLength, 0, 0);
            corners[2] = corners[0] + cv::Point3f(markerLength, -markerLength, 0);
            corners[3] = corners[0] + cv::Point3f(0, -markerLength, 0);
            objectPoints.push_back(corners);
        }
    }

    return objectPoints;
}

int cv_aruco_estimatePoseBoard(cv::InputArrayOfArrays _corners, cv::InputArray _ids,
    const cv::Ptr < cv::aruco::Board >& board,
    cv::InputArray _cameraMatrix, cv::InputArray _distCoeffs,
    cv::InputOutputArray _rvec, cv::InputOutputArray _tvec,
    bool useExtrinsicGuess = false)
{

    CV_Assert(_corners.total() == _ids.total());

    // get object and image points for the solvePnP function
    cv::Mat objPoints, imgPoints;
    getBoardObjectAndImagePoints(board, _corners, _ids, objPoints, imgPoints);

    CV_Assert(imgPoints.total() == objPoints.total());

    if (objPoints.total() == 0) // 0 of the detected markers in board
        return 0;

    cv::solvePnP(objPoints, imgPoints, _cameraMatrix, _distCoeffs, _rvec, _tvec, useExtrinsicGuess);

    // divide by four since all the four corners are concatenated in the array for each marker
    return (int)objPoints.total() / 4;
}

int main(int argc, char **argv)
{
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about(about);

    if (parser.get<bool>("h")) {
        parser.printMessage();
        return 0;
    }

    int dictionaryId = parser.get<int>("d");
    int wait_time = 10;

    cv::String videoInput = "0";
    cv::VideoCapture in_video;
    if (parser.has("v")) {
        videoInput = parser.get<cv::String>("v");
        if (videoInput.empty()) {
            parser.printMessage();
            return 1;
        }
        char* end = nullptr;
        int source = static_cast<int>(std::strtol(videoInput.c_str(), &end, \
            10));
        if (!end || end == videoInput.c_str()) {
            in_video.open(videoInput); // url
        } else {
            in_video.open(source); // id
        }
    } else {
        in_video.open(0);
    }

    if (!parser.check()) {
        parser.printErrors();
        return 1;
    }

    if (!in_video.isOpened()) {
        std::cerr << "failed to open video input: " << videoInput << std::endl;
        return 1;
    }

    cv::Mat image, image_copy;
    cv::Mat camera_matrix, dist_coeffs;
    std::ostringstream vector_to_marker;

    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary( \
        cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    cv::FileStorage fs("calibration_params.yml", cv::FileStorage::READ);

    fs["camera_matrix"] >> camera_matrix;
    fs["distortion_coefficients"] >> dist_coeffs;

    std::cout << "camera_matrix\n" << camera_matrix << std::endl;
    std::cout << "\ndist coeffs\n" << dist_coeffs << std::endl;


    auto length = 0.04;
    auto x_separation = 0.205 + length;
    auto y_separation = 0.345 + length;

#if 1
    auto objectPoints = createObjectPoints(3, 2, length, x_separation, y_separation);
    std::vector<int> boardIds({ 2, 3, 4, 5, 6, 7 });
    auto board = cv::aruco::Board::create(objectPoints, dictionary, boardIds);
#else
    auto board = cv::aruco::GridBoard::create(3, 2, length, x_separation, dictionary, 2);
#endif

#if 1
    cv::Mat board_image;
    cv::aruco::drawPlanarBoard(board, cv::Size(640, 480), board_image);
    cv::rectangle(board_image, cv::Point(0, 0), cv::Point(board_image.size().width, board_image.size().height), cv::Scalar(0, 255, 0), 10);
    imshow("Board", board_image);
#endif
    
    while (in_video.grab())
    {
        in_video.retrieve(image);
        
        image.copyTo(image_copy);

        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f> > corners;
        cv::aruco::detectMarkers(image, dictionary, corners, ids);

        // if at least one marker detected
        if (ids.size() > 0)
        {
            cv::aruco::drawDetectedMarkers(image_copy, corners, ids);
            
            cv::Vec3d rvecs, tvecs;
            cv_aruco_estimatePoseBoard(corners, ids, board, camera_matrix, dist_coeffs, rvecs, tvecs);

            cv::aruco::drawAxis(image_copy, camera_matrix, dist_coeffs, rvecs, tvecs, 0.1);
        }

        imshow("Pose estimation", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27)
            break;
    }

    in_video.release();

    return 0;
}
