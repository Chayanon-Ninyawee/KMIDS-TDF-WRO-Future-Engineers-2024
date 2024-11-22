#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "utils/lidarController.h"
#include "utils/lidarDataProcessor.h"
#include "utils/dataSaver.h"

const int WIDTH = 1200;
const int HEIGHT = 1200;
const float LIDAR_SCALE = 180.0;

const cv::Point CENTER(WIDTH/2, HEIGHT/2);



int main() {
    cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);

    // Load all scan data from file
    std::vector<std::vector<lidarController::NodeData>> allScanData;
    std::vector<bno055_accel_float_t> allAccelData;
    std::vector<bno055_euler_float_t> allEulerData;
    std::vector<cv::Mat> allIm;
    DataSaver::loadLogData("log/logData2.bin", allScanData, allAccelData, allEulerData, allIm);

    if (allScanData.empty() || allAccelData.empty() || allEulerData.empty()) {
        std::cerr << "No scan data found in file or failed to load." << std::endl;
        return -1;
    }

    // Iterate through all scan data and print each scan
    for (size_t i = 0; i < allScanData.size(); ++i) {
        cv::Mat im = allIm[i];

        bno055_accel_float_t accelData = allAccelData[i];
        bno055_euler_float_t eulerData = allEulerData[i];

        printf("Accel - X: %f, Y: %f, Z: %f\n", accelData.x, accelData.y, accelData.z);
        printf("Euler - H: %f, R: %f, P: %f\n", eulerData.h, eulerData.r, eulerData.p);


        cv::Mat binaryImage = lidarDataToImage(allScanData[i], WIDTH, HEIGHT, LIDAR_SCALE);
        cv::Mat outputImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
        cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);

        auto angle = fmod(eulerData.h - allEulerData[0].h + 360.0f, 360.0f);

        auto lines = detectLines(binaryImage);
        auto combinedLines = combineAlignedLines(lines);
        auto wallDirections = analyzeWallDirection(combinedLines, angle, CENTER);

        drawAllLines(outputImage, combinedLines, wallDirections);

        
        Direction direction = NORTH;

        if (angle >= 337.5 || angle < 22.5) direction = NORTH;
        else if (angle >= 22.5 && angle < 112.5) direction = EAST;
        else if (angle >= 112.5 && angle < 202.5) direction = SOUTH;
        else if (angle >= 202.5 && angle < 292.5) direction = WEST;
        else direction = NORTH; // Handles wrap-around for safety

        auto trafficLightPoints = detectTrafficLight(binaryImage, combinedLines, wallDirections, COUNTER_CLOCKWISE, direction);

        for (const auto point : trafficLightPoints) {
            cv::circle(outputImage, point, 10, cv::Scalar(255, 120, 255), cv::FILLED);
        }

        cv::circle(outputImage, CENTER, 10, cv::Scalar(0, 120, 255), cv::FILLED);

        // cv::Mat noWallBinaryImage = binaryImage.clone();
        // for (const auto& line : combinedLines) {
        //     // Extract the start and end points from the cv::Vec4i line
        //     cv::Point start(line[0], line[1]);
        //     cv::Point end(line[2], line[3]);

        //     // Draw a thick black line on the noWallBinaryImage
        //     cv::line(noWallBinaryImage, start, end, cv::Scalar(0), 40);
        // }
        // cv::cvtColor(noWallBinaryImage, outputImage, cv::COLOR_GRAY2BGR);

        for (size_t i = 0; i < combinedLines.size(); ++i) {
            cv::Vec4i line = combinedLines[i];
            printf("Line: %d, (%d, %d), (%d, %d), angle: %.2f\n", i, line[0], line[1], line[2], line[3], calculateAngle(line));
        }

        cv::imshow("LIDAR Hough Lines", outputImage);

        char key = cv::waitKey(100);
        if (key == 'q') {
            break;
        }
    }

    return 0;
}
