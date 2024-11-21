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
const float SCALE = 180.0;

const cv::Point CENTER(WIDTH/2, HEIGHT/2);

// Draw all lines with different colors based on direction (NORTH, EAST, SOUTH, WEST)
void drawAllLines(const std::vector<cv::Vec4i> &lines, cv::Mat &outputImage, double gyroYaw) {
    // Analyze the direction of each wall using the analyzeWallDirection function
    std::vector<Direction> wallDirections = analyzeWallDirection(lines, gyroYaw, CENTER);

    for (size_t i = 0; i < lines.size(); ++i) {
        cv::Vec4i line = lines[i];
        Direction direction = wallDirections[i];  // Get the direction of the current line

        // Determine the color based on the direction
        cv::Scalar color;
        if (direction == NORTH) {
            color = cv::Scalar(0, 0, 255); // Red for NORTH
        } else if (direction == EAST) {
            color = cv::Scalar(0, 255, 0); // Green for EAST
        } else if (direction == SOUTH) {
            color = cv::Scalar(255, 0, 0); // Blue for SOUTH
        } else if (direction == WEST) {
            color = cv::Scalar(255, 255, 0); // Yellow for WEST
        }

        // Draw the line with the determined color
        cv::line(outputImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), color, 2, cv::LINE_AA);
    }
}


int main() {
    cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);

    // Load all scan data from file
    std::vector<std::vector<lidarController::NodeData>> allScanData;
    std::vector<bno055_accel_float_t> allAccelData;
    std::vector<bno055_euler_float_t> allEulerData;
    DataSaver::loadLogData("log/logData.bin", allScanData, allAccelData, allEulerData);

    if (allScanData.empty() || allAccelData.empty() || allEulerData.empty()) {
        std::cerr << "No scan data found in file or failed to load." << std::endl;
        return -1;
    }

    // Iterate through all scan data and print each scan
    for (size_t i = 0; i < allScanData.size(); ++i) {
        bno055_accel_float_t accelData = allAccelData[i];
        bno055_euler_float_t eulerData = allEulerData[i];

        printf("Accel - X: %f, Y: %f, Z: %f\n", accelData.x, accelData.y, accelData.z);
        printf("Euler - H: %f, R: %f, P: %f\n", eulerData.h, eulerData.r, eulerData.p);


        cv::Mat binaryImage = lidarDataToImage(allScanData[i], WIDTH, HEIGHT, SCALE);
        cv::Mat outputImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
        cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);

        auto lines = detectLines(binaryImage);
        auto combined_lines = combineAlignedLines(lines);
        drawAllLines(combined_lines, outputImage, fmod(eulerData.h - 259.0f + 360.0f, 360.0f));

        for (size_t i = 0; i < combined_lines.size(); ++i) {
            cv::Vec4i line = combined_lines[i];
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
