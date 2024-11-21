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


        cv::Mat binaryImage = lidarDataToImage(allScanData[i], WIDTH, HEIGHT, SCALE);
        cv::Mat outputImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
        cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);

        auto lines = detectLines(binaryImage);
        auto combined_lines = combineAlignedLines(lines);
        drawAllLines(combined_lines, outputImage, fmod(eulerData.h - allEulerData[0].h + 360.0f, 360.0f));

        cv::Mat dilatedBinaryImage = binaryImage.clone();

        // Create a structuring element (kernel) of the specified size
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20));

        // Apply dilation to the binary image
        cv::dilate(binaryImage, dilatedBinaryImage, kernel);

        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(dilatedBinaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        std::vector<cv::Point> pillarPoints;
        for (const auto& contour : contours) {
            double area = cv::contourArea(contour);

            if (area >= 450 && area <= 700) {
                cv::Moments moments = cv::moments(contour);

                int centroidX = static_cast<int>(moments.m10 / moments.m00);
                int centroidY = static_cast<int>(moments.m01 / moments.m00);

                cv::Point point(centroidX, centroidY);

                for (const auto& line : combined_lines) {
                    cv::Vec4i extendedLine = extendLine(line, 1 + (30.0f / lineLength(line)));
                    if (pointToLineSegmentDistance(point, extendedLine) > 40) {
                        pillarPoints.push_back(point);
                    }
                }
            }
        }

        for (const auto point : pillarPoints) {
            cv::circle(outputImage, point, 10, cv::Scalar(255, 120, 255), cv::FILLED);
        }

        cv::circle(outputImage, CENTER, 10, cv::Scalar(0, 120, 255), cv::FILLED);

        // cv::Mat noWallBinaryImage = binaryImage.clone();
        // for (const auto& line : combined_lines) {
        //     // Extract the start and end points from the cv::Vec4i line
        //     cv::Point start(line[0], line[1]);
        //     cv::Point end(line[2], line[3]);

        //     // Draw a thick black line on the noWallBinaryImage
        //     cv::line(noWallBinaryImage, start, end, cv::Scalar(0), 40);
        // }
        // cv::cvtColor(noWallBinaryImage, outputImage, cv::COLOR_GRAY2BGR);

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
