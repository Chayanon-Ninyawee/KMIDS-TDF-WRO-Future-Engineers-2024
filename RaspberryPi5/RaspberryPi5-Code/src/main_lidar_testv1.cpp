#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "i2c_master.h"
#include "lidarController.h"

const uint8_t PICO_ADDRESS = 0x39;

bool isRunning = true;

void interuptHandler(int signum) {
    isRunning = false;
}

// Convert LIDAR data to an OpenCV image for Hough Line detection
cv::Mat lidarDataToImage(const std::vector<lidarController::NodeData> &data, int width, int height, float scale) {
    cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1);  // Grayscale image for binary line detection
    cv::Point center(width / 2, height / 2);

    for (const auto &point : data) {
        if (point.distance < 0.005)
            continue;
        if (point.distance > 3.200)
            continue;
        if (point.angle > 5 && point.angle < 175 && point.distance > 0.700)
            continue;

        float angle_rad = point.angle * CV_PI / 180.0;
        int x = static_cast<int>(center.x + point.distance * scale * cos(angle_rad));
        int y = static_cast<int>(center.y + point.distance * scale * sin(angle_rad));

        int radius = static_cast<int>(std::max(1.0, point.distance * scale * 0.011));  // Adjust 0.01 factor as needed

        if (x >= 0 && x < width && y >= 0 && y < height) {
            cv::circle(image, cv::Point(x, y), radius, cv::Scalar(255), -1);  // Draw circle at the point
        }
    }
    return image;
}

// Detect lines using Hough Transform
std::vector<cv::Vec4i> detectLines(const cv::Mat &binaryImage) {
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(binaryImage, lines, 1, CV_PI / 180, 50, 50, 10);
    return lines;
}

// Helper function to calculate the angle of a line in degrees
double calculateAngle(const cv::Vec4i &line) {
    double angle = atan2(line[3] - line[1], line[2] - line[0]) * 180.0 / CV_PI;
    angle = std::fmod(angle + 360.0, 360.0); // Map to [0, 360)
    return angle > 180.0 ? angle - 180.0 : angle; // Map to [0, 180)
}

// Calculate perpendicular distance
double pointLinePerpendicularDistance(const cv::Point2f& pt, const cv::Vec4i& line) {
    cv::Point2f lineStart(line[0], line[1]);
    cv::Point2f lineEnd(line[2], line[3]);
    double lineLength = cv::norm(lineEnd - lineStart);
    return std::abs((lineEnd.y - lineStart.y) * pt.x - (lineEnd.x - lineStart.x) * pt.y +
                    lineEnd.x * lineStart.y - lineEnd.y * lineStart.x) /
           lineLength;
}

// Check alignment and collinearity
bool areLinesAligned(const cv::Vec4i& line1, const cv::Vec4i& line2, double angleThreshold, double collinearThreshold) {
    double angle1 = calculateAngle(line1);
    double angle2 = calculateAngle(line2);

    if (std::abs(angle1 - angle2) > angleThreshold) {
        return false;
    }

    cv::Point2f start2(line2[0], line2[1]);
    cv::Point2f end2(line2[2], line2[3]);

    if (pointLinePerpendicularDistance(start2, line1) > collinearThreshold ||
        pointLinePerpendicularDistance(end2, line1) > collinearThreshold) {
        return false;
    }

    return true;
}

std::vector<cv::Vec4i> combineAlignedLines(std::vector<cv::Vec4i> lines, double angleThreshold = 5.0, double collinearThreshold = 10.0)
{
    // Ensure each line has its first point to the left (smaller x-coordinate) of the second point
    auto normalizeLine = [](cv::Vec4i& line) {
        if (line[0] > line[2] || (line[0] == line[2] && line[1] > line[3])) {
            std::swap(line[0], line[2]);
            std::swap(line[1], line[3]);
        }
    };

    for (auto& line : lines) {
        normalizeLine(line);
    }

    bool merged;

    do {
        merged = false;
        std::vector<cv::Vec4i> newLines;
        std::vector<bool> used(lines.size(), false);

        for (size_t i = 0; i < lines.size(); ++i) {
            if (used[i])
                continue;

            cv::Vec4i currentLine = lines[i];
            cv::Point2f start(currentLine[0], currentLine[1]);
            cv::Point2f end(currentLine[2], currentLine[3]);

            // Calculate direction vector of the current line
            cv::Point2f direction(end.x - start.x, end.y - start.y);
            double magnitude = cv::norm(direction);
            direction.x /= magnitude;
            direction.y /= magnitude;

            // Try to merge this line with others
            for (size_t j = i + 1; j < lines.size(); ++j) {
                if (used[j])
                    continue;

                cv::Vec4i otherLine = lines[j];

                if (areLinesAligned(currentLine, otherLine, angleThreshold, collinearThreshold)) {
                    cv::Point2f otherStart(otherLine[0], otherLine[1]);
                    cv::Point2f otherEnd(otherLine[2], otherLine[3]);

                    // Project all endpoints onto the line's direction
                    std::vector<cv::Point2f> points = {start, end, otherStart, otherEnd};
                    auto projection = [&](const cv::Point2f& pt) {
                        return (pt.x - start.x) * direction.x + (pt.y - start.y) * direction.y;
                    };

                    // Find min and max projections
                    double minProj = projection(points[0]);
                    double maxProj = projection(points[0]);
                    cv::Point2f minPoint = points[0];
                    cv::Point2f maxPoint = points[0];

                    for (const auto& pt : points) {
                        double proj = projection(pt);
                        if (proj < minProj) {
                            minProj = proj;
                            minPoint = pt;
                        }
                        if (proj > maxProj) {
                            maxProj = proj;
                            maxPoint = pt;
                        }
                    }

                    // Update start and end points
                    start = minPoint;
                    end = maxPoint;

                    used[j] = true; // Mark as combined
                    merged = true;  // Signal a merge occurred
                }
            }

            // Ensure the merged line is normalized
            cv::Vec4i combinedLine(start.x, start.y, end.x, end.y);
            normalizeLine(combinedLine);
            newLines.push_back(combinedLine);
        }

        // Update lines with the newly combined lines
        lines = std::move(newLines);

    } while (merged); // Repeat until no further merges are possible

    return lines;
}



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

int main(int argc, char **argv) {
    signal(SIGINT, interuptHandler);

    int fd = i2c_master_init(PICO_ADDRESS);

    i2c_master_send_command(fd, Command::RESTART);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    uint8_t calib[22] = {0x00, 0x00, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x10, 0xD3, 0x9E, 0xF5, 0xFE, 0x7F, 0x00, 0x00, 0x00, 0xD0, 0x0E, 0x1B, 0xFF, 0x7F};
    i2c_master_send_data(fd, i2c_slave_mem_addr::BNO055_CALIB_ADDR, calib, sizeof(calib));

    i2c_master_send_command(fd, Command::SKIP_CALIB);

    uint8_t status[i2c_slave_mem_addr::STATUS_SIZE] = {0};
    uint8_t logs[i2c_slave_mem_addr::LOGS_BUFFER_SIZE] = {0};
    while (not(status[0] & (1 << 1))) {
        i2c_master_read_data(fd, i2c_slave_mem_addr::STATUS_ADDR, status, sizeof(status));

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    uint8_t new_calib[22];
    i2c_master_read_data(fd, i2c_slave_mem_addr::BNO055_CALIB_ADDR, calib, sizeof(calib));

    for (int i = 0; i < sizeof(new_calib); i++) {
        printf("%x, ", new_calib[i]);
    }
    printf("\n");

    i2c_master_send_command(fd, Command::NO_COMMAND);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    lidarController::LidarController lidar;
    if (!lidar.initialize() || !lidar.startScanning()) {
        return -1;
    }

    const int width = 1200;
    const int height = 1200;
    const float scale = 180.0;

    cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);

    while (isRunning) {
        int64 start = cv::getTickCount();

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        auto lidarScanData = lidar.getScanData();
        // lidar.printScanData(lidarScanData);

        cv::Mat binaryImage = lidarDataToImage(lidarScanData, width, height, scale);
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

        char key = cv::waitKey(1);
        if (key == 'q') {
            break;
        }

        int64 end = cv::getTickCount();
        double duration = (end - start) / cv::getTickFrequency();
        double fps = 1.0 / duration;
        printf("FPS: %.3f, # of lines: %d\n", fps, combined_lines.size());
    }

    lidar.shutdown();
    cv::destroyAllWindows();

    return 0;
}
