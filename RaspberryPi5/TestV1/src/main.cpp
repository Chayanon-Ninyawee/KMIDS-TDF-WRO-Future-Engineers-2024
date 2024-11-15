#include <opencv2/opencv.hpp>
#include <vector>
#include <cmath>
#include <csignal>
#include <wiringPiI2C.h>
#include <chrono>
#include <thread>

#include "lidarController.h"

bool isRunning = true;

void interuptHandler(int signum)
{
  isRunning = false;
}

// Convert LIDAR data to an OpenCV image for Hough Line detection
cv::Mat lidarDataToImage(const std::vector<lidarController::NodeData> &data, int width, int height, float scale)
{
  cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1); // Grayscale image for binary line detection
  cv::Point center(width / 2, height / 2);

  for (const auto &point : data)
  {
    if (point.distance < 0.005)
      continue;
    if (point.distance > 3.200)
      continue;
    if (point.angle > 5 && point.angle < 175 && point.distance > 0.700)
      continue;

    float angle_rad = point.angle * CV_PI / 180.0;
    int x = static_cast<int>(center.x + point.distance * scale * cos(angle_rad));
    int y = static_cast<int>(center.y + point.distance * scale * sin(angle_rad));

    int radius = static_cast<int>(std::max(1.0, point.distance * scale * 0.011)); // Adjust 0.01 factor as needed

    if (x >= 0 && x < width && y >= 0 && y < height)
    {
      cv::circle(image, cv::Point(x, y), radius, cv::Scalar(255), -1); // Draw circle at the point
    }
  }
  return image;
}

// Detect lines using Hough Transform
std::vector<cv::Vec4i> detectLines(const cv::Mat &binaryImage)
{
  std::vector<cv::Vec4i> lines;
  cv::HoughLinesP(binaryImage, lines, 1, CV_PI / 180, 50, 50, 10);
  return lines;
}

// Helper function to calculate the angle of a line in degrees
double calculateAngle(const cv::Vec4i &line)
{
  return atan2(line[3] - line[1], line[2] - line[0]) * 180.0 / CV_PI;
}

// Helper function to calculate the perpendicular distance from a point to a line
double pointLinePerpendicularDistance(const cv::Point2f &pt, const cv::Vec4i &line)
{
  cv::Point2f lineStart(line[0], line[1]);
  cv::Point2f lineEnd(line[2], line[3]);
  double lineLength = cv::norm(lineEnd - lineStart);
  return std::abs((lineEnd.y - lineStart.y) * pt.x - (lineEnd.x - lineStart.x) * pt.y + lineEnd.x * lineStart.y - lineEnd.y * lineStart.x) / lineLength;
}

// Helper function to check if two lines are close in angle and collinear
bool areLinesAligned(const cv::Vec4i &line1, const cv::Vec4i &line2, double angleThreshold, double collinearThreshold)
{
  // Check if the angles are similar
  double angle1 = calculateAngle(line1);
  double angle2 = calculateAngle(line2);

  if (std::abs(angle1 - angle2) >= angleThreshold and std::abs(std::abs(angle1 - angle2) - 180) >= angleThreshold) // Add and std::abs(std::abs(angle1 - angle2) - 180) >= angleThreshold to prevent the situation (where angle is 90 and -90 which is the same) to be reconize as different angle
  {
    return false;
  }

  // Check if the lines are collinear by measuring the perpendicular distance from each endpoint of line2 to line1
  cv::Point2f start2(line2[0], line2[1]);
  cv::Point2f end2(line2[2], line2[3]);

  if (pointLinePerpendicularDistance(start2, line1) >= collinearThreshold) {
    return false;
  }
  if (pointLinePerpendicularDistance(end2, line1) >= collinearThreshold) {
    return false;
  }

  // If no points on line2 are within the collinear threshold, return true
  return true;
}

// Combine aligned lines continuously until no further merging is possible
std::vector<cv::Vec4i> combineAlignedLines(std::vector<cv::Vec4i> lines, double angleThreshold = 5.0, double collinearThreshold = 10.0)
{
  bool merged;

  do
  {
    merged = false;
    std::vector<cv::Vec4i> newLines;

    // Mark lines as used once combined
    std::vector<bool> used(lines.size(), false);

    for (size_t i = 0; i < lines.size(); ++i)
    {
      if (used[i])
        continue;

      cv::Vec4i currentLine = lines[i];
      cv::Point2f start(currentLine[0], currentLine[1]);
      cv::Point2f end(currentLine[2], currentLine[3]);

      // Try to combine this line with others
      for (size_t j = i + 1; j < lines.size(); ++j)
      {
        if (used[j])
          continue;

        cv::Vec4i otherLine = lines[j];

        if (areLinesAligned(currentLine, otherLine, angleThreshold, collinearThreshold))
        {
          cv::Point2f otherStart(otherLine[0], otherLine[1]);
          cv::Point2f otherEnd(otherLine[2], otherLine[3]);

          // TODO: Fix this and figure out why this doesn't work
          // // Extend the line segment to include the new line
          // start.x = std::min(start.x, otherStart.x);
          // start.y = std::min(start.y, otherStart.y);
          // end.x = std::max(end.x, otherEnd.x);
          // end.y = std::max(end.y, otherEnd.y);

          used[j] = true; // Mark as combined
          merged = true;  // Indicate that a merge has occurred
        }
      }

      newLines.push_back(cv::Vec4i(start.x, start.y, end.x, end.y));
    }

    lines = std::move(newLines); // Update the lines vector with newly combined lines

  } while (merged); // Repeat until no more merges occur

  return lines;
}

// Draw all lines with different colors
void drawAllLines(const std::vector<cv::Vec4i> &lines, cv::Mat &outputImage)
{
  for (size_t i = 0; i < lines.size(); ++i)
  {
    // Generate a bright, contrasting color by using modulo values to avoid dark colors
    int r = ((i * 73) % 156) + 100;  // Red component, adjusted to stay above 100
    int g = ((i * 89) % 156) + 100;  // Green component, adjusted to stay above 100
    int b = ((i * 113) % 156) + 100; // Blue component, adjusted to stay above 100

    cv::Scalar color(0, 255, 0); // Ensure colors are bright and non-black
    cv::Vec4i line = lines[i];
    cv::line(outputImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), color, 2, cv::LINE_AA);
  }
}


int main(int argc, char **argv)
{
  signal(SIGINT, interuptHandler);

  int fd = wiringPiI2CSetup(0x39);
  if (fd == -1) {
    printf("Failed to initialize I2C communication.\n");
    return -1;
  }
  printf("I2C communication successfully initialized.\n");

  // wiringPiI2CWrite(fd, 0x02);
  uint8_t calib[23] = {2, 0xfe, 0xff, 0x00, 0x00, 0xff, 0xff, 0xf0, 0xff, 0xf2, 0xff, 0xe0, 0xff, 0xe8, 0x03, 0x50, 0xfe, 0xe7, 0xff, 0xd0, 0x07, 0xcd, 0x01};
  wiringPiI2CRawWrite(fd, calib, sizeof(calib));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  uint8_t cmd[2] = {0, 0x03};
  wiringPiI2CRawWrite(fd, cmd, sizeof(cmd));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  uint8_t status[1] = {0};
  while (not (status[0] & (1 << 1))) {
    wiringPiI2CReadBlockData(fd, 1, status, sizeof(status));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  printf("%x\n", status[0]);

  uint8_t test[22] = {0};
  wiringPiI2CReadBlockData(fd, 2, test, sizeof(test));

  for (int i = 0; i < sizeof(test); i++) {
    printf("%x, ", test[i]);
  }
  printf("\n");



  // lidarController::LidarController lidar;
  // if (!lidar.initialize() || !lidar.startScanning())
  // {
  //   return -1;
  // }

  // const int width = 1200;
  // const int height = 1200;
  // const float scale = 180.0;

  // cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);

  while (isRunning)
  {

    // int64 start = cv::getTickCount();
    // auto lidarScanData = lidar.getScanData();
    // // lidar.printScanData(lidarScanData);

    // cv::Mat binaryImage = lidarDataToImage(lidarScanData, width, height, scale);
    // cv::Mat outputImage = cv::Mat::zeros(height, width, CV_8UC3);
    // cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);

    // auto lines = detectLines(binaryImage);
    // auto combined_lines = combineAlignedLines(lines);
    // drawAllLines(combined_lines, outputImage);

    // for (size_t i = 0; i < combined_lines.size(); ++i)
    // {
    //   cv::Vec4i line = combined_lines[i];
    //   printf("Line: %d, (%d, %d), (%d, %d), angle: %.2f\n", i, line[0], line[1], line[2], line[3], calculateAngle(line));
    // }

    // cv::imshow("LIDAR Hough Lines", outputImage);

    // char key = cv::waitKey(1);
    // if (key == 'q')
    // {
    //   break;
    // }

    // int64 end = cv::getTickCount();
    // double duration = (end - start) / cv::getTickFrequency();
    // double fps = 1.0 / duration;
    // printf("FPS: %.3f, # of lines: %d\n", fps, combined_lines.size());
  }

  // lidar.shutdown();
  // cv::destroyAllWindows();

  return 0;
}
