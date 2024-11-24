#ifndef LIDAR_DATA_PROCESSOR_H
#define LIDAR_DATA_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <vector>

#include "lidarController.h"

#include "direction.h"
#include "imageProcessor.h"

struct BlockInfo {
    float angle;  // Angle in degrees
    int size;
    Color color;  // Color of the traffic light
};

struct ProcessedTrafficLight {
    cv::Point point; // Location of block
    Color color;  // Color of the traffic light
};

// Converts LIDAR data to a grayscale OpenCV image for Hough Line detection
cv::Mat lidarDataToImage(const std::vector<lidarController::NodeData> &data, int width, int height, float scale);

float convertLidarDistanceToActualDistance(int scale, double lidarDistance);

// Detects lines using the Hough Transform
std::vector<cv::Vec4i> detectLines(const cv::Mat &binaryImage);

double lineLength(const cv::Vec4i& line);

// Calculates the angle of a line in degrees
double calculateAngle(const cv::Vec4i &line);

// Calculates the perpendicular distance from a point to a line
double pointLinePerpendicularDistance(const cv::Point2f& pt, const cv::Vec4i& line);

double pointLinePerpendicularDirection(const cv::Point2f& pt, const cv::Vec4i& line);

double pointToLineSegmentDistance(const cv::Point2f& P, const cv::Vec4i& lineSegment);

cv::Vec4i extendLine(const cv::Vec4i& line, double factor);

// Checks if two lines are aligned and collinear within specified thresholds
bool areLinesAligned(const cv::Vec4i& line1, const cv::Vec4i& line2, double angleThreshold, double collinearThreshold);

// Combines aligned and collinear lines into single lines
std::vector<cv::Vec4i> combineAlignedLines(std::vector<cv::Vec4i> lines, double angleThreshold = 12.0, double collinearThreshold = 10.0);

// Function to analyze the combined lines with gyro data and classify them as NORTH, EAST, SOUTH, WEST
std::vector<Direction> analyzeWallDirection(const std::vector<cv::Vec4i>& combinedLines, float gyroYaw, const cv::Point& center);

std::vector<cv::Point> detectTrafficLight(const cv::Mat& binaryImage, const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, TurnDirection turnDirection, Direction direction);

std::vector<ProcessedTrafficLight> processTrafficLight(
    const std::vector<cv::Point>& trafficLightPoints, 
    const std::vector<BlockInfo>& blockAngles,
    const cv::Point& center
);

TurnDirection lidarDetectTurnDirection(const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, Direction direction);

float convertLidarDistanceToActualDistance(int scale, double lidarDistance);

void drawAllLines(cv::Mat &outputImage, const std::vector<cv::Vec4i> &lines, const std::vector<Direction> &wallDirections);

void drawTrafficLights(cv::Mat& outputImage, const std::vector<ProcessedTrafficLight>& processedBlocks);

#endif // LIDAR_DATA_PROCESSOR_H
