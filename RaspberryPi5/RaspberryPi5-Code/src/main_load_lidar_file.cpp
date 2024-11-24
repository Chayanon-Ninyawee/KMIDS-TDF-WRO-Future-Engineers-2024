// Make so that the video can be watch frame by frame

#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "utils/lidarController.h"
#include "utils/lidarDataProcessor.h"
#include "utils/imageProcessor.h"
#include "utils/dataSaver.h"

const int LIDAR_WIDTH = 1200;
const int LIDAR_HEIGHT = 1200;
const float LIDAR_SCALE = 180.0;

const cv::Point CENTER(LIDAR_WIDTH/2, LIDAR_HEIGHT/2);

uint32_t camWidth = 1280;
uint32_t camHeight = 720;





size_t frameIndex = 0; // Track the current frame
bool playVideo = false;
char key = 0;





// Global variable to store the clicked point coordinates
cv::Point clickPoint(-1, -1);

// Mouse callback function to capture the click event
void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        // Store the clicked point
        clickPoint = cv::Point(x, y);
    }
}






void drawRadialLines(cv::Mat &image, const cv::Point &center, float angle, int length, cv::Scalar color, int thickness = 1) {
    // Convert angle to radians
    double theta = angle * CV_PI / 180.0;

    // Calculate the end point of the line
    int endX = static_cast<int>(center.x + length * std::sin(theta));
    int endY = static_cast<int>(center.y - length * std::cos(theta));

    // Draw the line from the center to the end point
    cv::line(image, center, cv::Point(endX, endY), color, thickness);
}





int main() {
    cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("LIDAR Hough Lines", onMouse, nullptr);

    // Load all scan data from file
    std::vector<std::vector<lidarController::NodeData>> allScanData;
    std::vector<bno055_accel_float_t> allAccelData;
    std::vector<bno055_euler_float_t> allEulerData;
    std::vector<cv::Mat> allCameraImage;
    DataSaver::loadLogData("log/logData2.bin", allScanData, allAccelData, allEulerData, allCameraImage);

    if (allScanData.empty() || allAccelData.empty() || allEulerData.empty() || allCameraImage.empty()) {
        std::cerr << "No scan data found in file or failed to load." << std::endl;
        return -1;
    }



    printf("Press Any Key to Start\n");  
    getchar();



    bno055_accel_float_t initialAccelData = allAccelData[0];
    bno055_euler_float_t initialEulerData = allEulerData[0];

    while (true) {
        if (playVideo) {
            key = cv::waitKey(100); // Play at ~10 FPS
            frameIndex++;           // Move forward in play mode
        } else {
            key = cv::waitKey(100); // Wait indefinitely for user input
        }

        if (key == 'p') playVideo = !playVideo; // Toggle play/pause
        if (key == 'a' && !playVideo && frameIndex > 0) frameIndex--; // Rewind 1 frame
        if (key == 'd' && !playVideo) frameIndex++; // Forward 1 frame
        if (key == 'q') break; // Exit loop

        // Boundary check during play
        if (frameIndex >= allScanData.size()) frameIndex = allScanData.size() - 1;




        cv::Mat rawCameraImage = allCameraImage[frameIndex];
        cv::Mat cameraImage(rawCameraImage.rows * 2, rawCameraImage.cols, rawCameraImage.type());
        cameraImage.setTo(cv::Scalar(0, 0, 0)); // black in BGR
        rawCameraImage.copyTo(cameraImage(cv::Rect(0, rawCameraImage.rows, rawCameraImage.cols, rawCameraImage.rows)));


        // cv::Mat cameraImage;
        // cv::flip(allCameraImage[frameIndex], cameraImage, 1);

        bno055_accel_float_t accelData = allAccelData[frameIndex];
        bno055_euler_float_t eulerData = allEulerData[frameIndex];

        auto lidarScanData = allScanData[frameIndex];


        cv::Mat binaryImage = lidarDataToImage(lidarScanData, LIDAR_WIDTH, LIDAR_HEIGHT, LIDAR_SCALE);
        // cv::Mat binaryImage;
        // cv::flip(lidarDataToImage(lidarScanData, LIDAR_WIDTH, LIDAR_HEIGHT, LIDAR_SCALE), binaryImage, 1);
        cv::Mat lidarOutputImage = cv::Mat::zeros(LIDAR_HEIGHT, LIDAR_WIDTH, CV_8UC3);
        cv::cvtColor(binaryImage, lidarOutputImage, cv::COLOR_GRAY2BGR);



        auto angle = fmod(eulerData.h - initialEulerData.h + 360.0f, 360.0f);
        // auto angle = fmod(-(eulerData.h - initialEulerData.h) + 360.0f + 360.0f, 360.0f);
        // float angle = 0.0;



        auto lines = detectLines(binaryImage);
        auto combinedLines = combineAlignedLines(lines);
        auto wallDirections = analyzeWallDirection(combinedLines, angle, CENTER);

        drawAllLines(lidarOutputImage, combinedLines, wallDirections);




        Direction direction = NORTH; // Placeholder for intended direction (work fine for testing)
        if (angle >= 315 || angle < 45) direction = NORTH;
        else if (angle >= 45 && angle < 135) direction = EAST;
        else if (angle >= 135 && angle < 225) direction = SOUTH;
        else if (angle >= 225 && angle < 315) direction = WEST;




        // auto trafficLightPoints = detectTrafficLight(binaryImage, combinedLines, wallDirections, COUNTER_CLOCKWISE, direction);
        auto trafficLightPoints = detectTrafficLight(binaryImage, combinedLines, wallDirections, COUNTER_CLOCKWISE, direction);

        TurnDirection turnDirection = lidarDetectTurnDirection(combinedLines, wallDirections, direction);




        for (const auto point : trafficLightPoints) {
            cv::circle(lidarOutputImage, point, 5, cv::Scalar(255, 120, 255), cv::FILLED);
        }

        cv::circle(lidarOutputImage, CENTER, 10, cv::Scalar(0, 120, 255), cv::FILLED);

        // cv::Mat filteredCameraImage = filterAllColors(cameraImage);
        auto cameraImageData = processImage(cameraImage);
        // auto filteredCameraImage = filterAllColors(cameraImage);
        cv::Mat processedImage = drawImageProcessingResult(cameraImageData, cameraImage);

        std::vector<BlockInfo> blockAngles;
        for (Block block : cameraImageData.blocks) {
            BlockInfo blockAngle;
            blockAngle.angle = pixelToAngle(block.x, camWidth, 20, 88.0f);
            blockAngle.size = block.size;
            blockAngle.color = block.color;
            blockAngles.push_back(blockAngle);
            
            // cv::Scalar color;
            // if (blockAngle.color == RED) {
            //     color = cv::Scalar(0, 0, 255);
            // } else {
            //     color = cv::Scalar(0, 255, 0);
            // }

            // drawRadialLines(lidarOutputImage, CENTER, blockAngle.angle, 800, color, 2);
        }
        auto processedTrafficLights = processTrafficLight(trafficLightPoints, blockAngles, CENTER);

        drawTrafficLights(lidarOutputImage, processedTrafficLights);


        cv::imshow("LIDAR Hough Lines", lidarOutputImage);

        // printf("turnDirection: %d\n", turnDirection);





        // Check if a mouse click was detected
        if (clickPoint.x != -1 && clickPoint.y != -1) {
            // Get the pixel color at the clicked point (in BGR format)
            cv::Vec3b pixelColor = cameraImage.at<cv::Vec3b>(clickPoint);

            // Convert the BGR color to HSV
            cv::Mat hsvImage;
            cv::cvtColor(cameraImage, hsvImage, cv::COLOR_BGR2HSV);
            cv::Vec3b hsvColor = hsvImage.at<cv::Vec3b>(clickPoint);

            // Print the HSV values
            std::cout << "HSV of clicked pixel (" << clickPoint.x << ", " << clickPoint.y << "): "
                      << "H = " << (int)hsvColor[0] << ", "
                      << "S = " << (int)hsvColor[1] << ", "
                      << "V = " << (int)hsvColor[2] << std::endl;

            // Reset the clickPoint
            clickPoint = cv::Point(-1, -1);
        }
    }

    return 0;
}