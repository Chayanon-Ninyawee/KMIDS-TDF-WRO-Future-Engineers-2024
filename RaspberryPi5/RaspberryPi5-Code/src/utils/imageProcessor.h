#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

const float CROP_PERCENT = 0.50;

// Define constants for color ranges and minimum contour areas
const cv::Scalar lowerBlueLine(100, 100, 150);
const cv::Scalar upperBlueLine(140, 220, 220);
const cv::Scalar lowerOrangeLine(4, 120, 180);
const cv::Scalar upperOrangeLine(16, 230, 255);
const cv::Scalar lowerRed1Light(0, 120, 185);
const cv::Scalar upperRed1Light(1, 205, 255);
const cv::Scalar lowerRed2Light(176, 120, 185);
const cv::Scalar upperRed2Light(180, 205, 255);
const cv::Scalar lowerGreen1Light(32, 75, 140);
const cv::Scalar upperGreen1Light(50, 135, 190);
const cv::Scalar lowerGreen2Light(50, 135, 190);
const cv::Scalar upperGreen2Light(50, 135, 190);
const cv::Scalar lowerPinkLight(165, 244, 200);
const cv::Scalar upperPinkLight(171, 255, 255);

const int minBlueLineArea = 37;
const int minOrangeLineArea = 37;
const int minRedLineArea = 300;
const int minGreenLineArea = 300;

enum Color {
    BLUE,
    ORANGE,
    RED,
    GREEN,
    PINK
};

// Struct to store block data
struct Block {
    int x;
    int y;
    int lowestY;
    int size;
    Color color;
};

// Struct to store the processing results
struct ImageProcessingResult {
    int blueLineY;
    int blueLineSize;
    int orangeLineY;
    int orangeLineSize;
    std::vector<Block> blocks;  // A vector to store block data
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



float pixelToAngle(int pixelX, int imageWidth, int centerOffset, float fov);



cv::Mat drawImageProcessingResult(const ImageProcessingResult &result, cv::Mat &image);

#endif // IMAGE_PROCESSOR_H
