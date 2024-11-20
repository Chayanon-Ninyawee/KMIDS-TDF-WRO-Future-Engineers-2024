#include "lidarDataProcessor.h"

#include <cmath>

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

std::vector<cv::Vec4i> combineAlignedLines(std::vector<cv::Vec4i> lines, double angleThreshold, double collinearThreshold)
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