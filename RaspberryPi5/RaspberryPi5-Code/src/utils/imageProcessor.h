#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

// Define constants for color ranges and minimum contour areas
const cv::Scalar lowerBlueLine(100, 100, 150);
const cv::Scalar upperBlueLine(140, 220, 220);
const cv::Scalar lowerOrangeLine(4, 120, 180);
const cv::Scalar upperOrangeLine(16, 230, 255);
const cv::Scalar lowerRed1Light(0, 180, 200);
const cv::Scalar upperRed1Light(1, 220, 255);
const cv::Scalar lowerRed2Light(177, 180, 200);
const cv::Scalar upperRed2Light(180, 255, 255);
const cv::Scalar lowerGreenLight(27, 70, 90);
const cv::Scalar upperGreenLight(50, 190, 250);
const cv::Scalar lowerPinkLight(165, 244, 200);
const cv::Scalar upperPinkLight(171, 255, 255);

const int minBlueLineArea = 500;
const int minOrangeLineArea = 500;
const int minRedLineArea = 500;
const int minGreenLineArea = 500;

// Struct to store the processing results
struct ImageProcessingResult {
    int blueLineY;
    int blueLineSize;
    int orangeLineY;
    int orangeLineSize;
    int closestBlockX;
    int closestBlockY;
    int closestBlockLowestY;
    int closestBlockSize;
    std::string closestBlockColor;
    int pinkX;
    int pinkY;
    int pinkSize;
};

// Function declarations
std::vector<cv::Point> getCoordinates(const cv::Mat &mask);
int getAverageX(const std::vector<cv::Point> &coordinates);
int getAverageY(const std::vector<cv::Point> &coordinates);
std::tuple<cv::Point, double, cv::Point> getCentroidAndArea(const std::vector<cv::Point> &contour);
ImageProcessingResult processImage(const cv::Mat &image);

// New function declaration
cv::Mat filterAllColors(const cv::Mat &image);

#endif // IMAGE_PROCESSOR_H
