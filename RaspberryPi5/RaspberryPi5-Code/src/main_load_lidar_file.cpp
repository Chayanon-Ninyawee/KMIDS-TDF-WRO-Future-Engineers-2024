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



// Draw all lines with different colors
void drawAllLines(const std::vector<cv::Vec4i> &lines, cv::Mat &outputImage) {
    for (size_t i = 0; i < lines.size(); ++i) {
        // Generate a bright, contrasting color by using modulo values to avoid dark colors
        int r = ((i * 73) % 156) + 100;   // Red component, adjusted to stay above 100
        int g = ((i * 89) % 156) + 100;   // Green component, adjusted to stay above 100
        int b = ((i * 113) % 156) + 100;  // Blue component, adjusted to stay above 100

        cv::Scalar color(0, 255, 0);  // Ensure colors are bright and non-black
        cv::Vec4i line = lines[i];
        cv::line(outputImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), color, 2, cv::LINE_AA);
    }
}


const int width = 1200;
const int height = 1200;
const float scale = 180.0;

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


        cv::Mat binaryImage = lidarDataToImage(allScanData[i], width, height, scale);
        cv::Mat outputImage = cv::Mat::zeros(height, width, CV_8UC3);
        cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);

        auto lines = detectLines(binaryImage);
        auto combined_lines = combineAlignedLines(lines);
        drawAllLines(combined_lines, outputImage);

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
