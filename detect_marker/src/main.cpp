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
const char* about = "Detect ArUco marker images";
const char* keys  =
        "{d        |16    | dictionary: DICT_4X4_50=0, DICT_4X4_100=1, "
        "DICT_4X4_250=2, DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, "
        "DICT_5X5_250=6, DICT_5X5_1000=7, DICT_6X6_50=8, DICT_6X6_100=9, "
        "DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12, DICT_7X7_100=13, "
        "DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL = 16}"
        "{h        |false | Print help }"
        "{v        |<none>| Custom video source, otherwise '0' }"
        ;
}

cv::Point2f transformPoint(cv::Mat transform, cv::Point2f p)
{
    std::vector<cv::Point3f> input;
    input.push_back(cv::Point3f(p.x, p.y, 1));
    std::vector<cv::Point3f> output;
    cv::transform(input, output, transform);
    return cv::Point2f(output[0].x / output[0].z, output[0].y / output[0].z);
}


static cv::Point2f cursor;
static void onMouse(int event, int x, int y, int, void*);


typedef std::vector<cv::Point2f> Corners;
typedef std::map<int, Corners> Markers;
static Markers detectMarkers(cv::InputArray image, const cv::Ptr<cv::aruco::Dictionary>& dictionary)
{
    Markers retv;

    std::vector<int> ids;
    std::vector<Corners> corners;
    cv::aruco::detectMarkers(image, dictionary, corners, ids);
    for (unsigned i = 0; i < ids.size(); ++i) {
        retv[ids[i]] = corners[i];
    }

    return retv;
}


static void drawMarkers(cv::InputOutputArray image, const Markers& markers)
{
    if (markers.empty()) {
        return;
    }

    std::vector<int> ids;
    std::transform(std::begin(markers), std::end(markers), std::back_inserter(ids), [](const auto& m) { return m.first; });

    std::vector<Corners> corners;
    std::transform(std::begin(markers), std::end(markers), std::back_inserter(corners), [](const auto& m) { return m.second; });

    cv::aruco::drawDetectedMarkers(image, corners, ids);
}


static void drawCursor(cv::InputOutputArray image, const cv::Point2f& position)
{
    auto rect = cv::Rect(position, cv::Point2f(position.x + 10, position.y + 10));
    cv::rectangle(image, rect, cv::Scalar(0, 0, 255), 2);
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

    cv::Ptr<cv::aruco::Dictionary> dictionary =
        cv::aruco::getPredefinedDictionary( \
        cv::aruco::PREDEFINED_DICTIONARY_NAME(dictionaryId));

    auto reference_image = cv::imread("reference.jpg", cv::IMREAD_COLOR);
    auto reference_markers = detectMarkers(reference_image, dictionary);
    drawMarkers(reference_image, reference_markers);
    imshow("Reference markers", reference_image);
    cv::setMouseCallback("Reference markers", onMouse, &cursor);

    std::array<int, 4> used_markers = { 2, 4, 7, 5 };

    Corners reference_points;
    std::transform(std::begin(used_markers), std::end(used_markers), std::back_inserter(reference_points), [&reference_markers](auto m) { return reference_markers[m][0]; });

    while (in_video.grab()) {
        cv::Mat image, image_copy;
        in_video.retrieve(image);
        image.copyTo(image_copy);

        auto markers = detectMarkers(image, dictionary);
        
        auto not_found = std::end(markers);
        if (markers.find(7) != not_found) {
            if (markers.find(5) != not_found) {
                cv::line(image_copy, markers[7][1], markers[5][0], cv::Scalar(0, 0, 255), 5);
            }
            if (markers.find(4) != not_found) {
                cv::line(image_copy, markers[7][1], markers[4][1], cv::Scalar(0, 0, 255), 5);
            }
        }
        
        if (std::all_of(std::begin(used_markers), std::end(used_markers), [&markers, &not_found](auto m) { return markers.find(m) != not_found; })) {
            Corners image_points;
            std::transform(std::begin(used_markers), std::end(used_markers), std::back_inserter(image_points), [&markers](auto m) { return markers[m][0]; });
            drawCursor(image_copy, transformPoint(cv::getPerspectiveTransform(reference_points, image_points), cursor));
        }

        drawMarkers(image_copy, markers);

        imshow("Detected markers", image_copy);
        char key = (char)cv::waitKey(wait_time);
        if (key == 27)
            break;
    }

    in_video.release();

    return 0;
}


static void onMouse(int event, int x, int y, int, void* user)
{
    // TODO: make atomic
    cv::Point2f& cursor = *reinterpret_cast<cv::Point2f*>(user);
    cursor.x = static_cast<float>(x);
    cursor.y = static_cast<float>(y);
}

