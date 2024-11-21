#ifndef LIDAR_DATA_PROCESSOR_H
#define LIDAR_DATA_PROCESSOR_H

#include <opencv2/opencv.hpp>
#include <vector>

#include "lidarController.h"

#include "direction.h"

// Converts LIDAR data to a grayscale OpenCV image for Hough Line detection
cv::Mat lidarDataToImage(const std::vector<lidarController::NodeData> &data, int width, int height, float scale);

// Detects lines using the Hough Transform
std::vector<cv::Vec4i> detectLines(const cv::Mat &binaryImage);

// Calculates the angle of a line in degrees
double calculateAngle(const cv::Vec4i &line);

// Calculates the perpendicular distance from a point to a line
double pointLinePerpendicularDistance(const cv::Point2f& pt, const cv::Vec4i& line);

// Checks if two lines are aligned and collinear within specified thresholds
bool areLinesAligned(const cv::Vec4i& line1, const cv::Vec4i& line2, double angleThreshold, double collinearThreshold);

// Combines aligned and collinear lines into single lines
std::vector<cv::Vec4i> combineAlignedLines(std::vector<cv::Vec4i> lines, double angleThreshold = 12.0, double collinearThreshold = 10.0);

// Function to analyze the combined lines with gyro data and classify them as NORTH, EAST, SOUTH, WEST
std::vector<Direction> analyzeWallDirection(const std::vector<cv::Vec4i>& combinedLines, float gyroYaw, const cv::Point& center);

float convertLidarDistanceToActualDistance(int scale, double lidarDistance);

#endif // LIDAR_DATA_PROCESSOR_H
