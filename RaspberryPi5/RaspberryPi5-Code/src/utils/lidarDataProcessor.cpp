#include "lidarDataProcessor.h"

#include <cmath>

// Convert LIDAR data to an OpenCV image for Hough Line detection
cv::Mat lidarDataToImage(const std::vector<lidarController::NodeData>& data, int width, int height, float scale) {
    cv::Mat image = cv::Mat::zeros(height, width, CV_8UC1);  // Grayscale image for binary line detection
    cv::Point center(width / 2, height / 2);

    for (const auto& point : data) {
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
std::vector<cv::Vec4i> detectLines(const cv::Mat& binaryImage) {
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(binaryImage, lines, 1, CV_PI / 180, 50, 50, 30);
    return lines;
}

double lineLength(const cv::Vec4i& line) {
    return cv::norm(cv::Point(line[2], line[3]) - cv::Point(line[0], line[1]));
}

// Helper function to calculate the angle of a line in degrees
double calculateAngle(const cv::Vec4i& line) {
    double angle = atan2(line[3] - line[1], line[2] - line[0]) * 180.0 / CV_PI;
    angle = std::fmod(angle + 360.0, 360.0);       // Map to [0, 360)
    return angle > 180.0 ? angle - 180.0 : angle;  // Map to [0, 180)
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

double pointToLineSegmentDistance(const cv::Point2f& P, const cv::Vec4i& lineSegment) {
    // Extract endpoints from the line segment
    cv::Point2f A(lineSegment[0], lineSegment[1]);
    cv::Point2f B(lineSegment[2], lineSegment[3]);

    // Vector from A to P
    cv::Point2f AP = P - A;
    // Vector from A to B
    cv::Point2f AB = B - A;

    // Squared length of AB
    double AB_squared = AB.dot(AB);

    // Handle case where A and B are the same point
    if (AB_squared == 0.0) {
        return cv::norm(P - A);
    }

    // Projection factor of P onto AB
    double t = AP.dot(AB) / AB_squared;

    // Clamp t to the range [0, 1]
    t = std::max(0.0, std::min(1.0, t));

    // Closest point on the line segment to P
    cv::Point2f closestPoint = A + t * AB;

    // Distance from P to the closest point
    return cv::norm(P - closestPoint);
}

cv::Vec4i extendLine(const cv::Vec4i& line, double factor) {
    cv::Point2f A(line[0], line[1]); // Start point
    cv::Point2f B(line[2], line[3]); // End point

    cv::Point2f AB = B - A;          // Line vector
    AB *= factor / cv::norm(AB);     // Scale vector by the factor

    cv::Point2f newA = A - AB;       // Extend backward
    cv::Point2f newB = B + AB;       // Extend forward

    return cv::Vec4i(cvRound(newA.x), cvRound(newA.y), cvRound(newB.x), cvRound(newB.y));
}

// Check alignment and collinearity
bool areLinesAligned(const cv::Vec4i& line1, const cv::Vec4i& line2, double angleThreshold, double collinearThreshold) {
    double angle1 = calculateAngle(line1);
    double angle2 = calculateAngle(line2);

    if (std::abs(angle1 - angle2) >= angleThreshold and std::abs(std::abs(angle1 - angle2) - 180) >= angleThreshold) {
        return false;
    }

    cv::Point2f start2(line2[0], line2[1]);
    cv::Point2f end2(line2[2], line2[3]);

    if (pointLinePerpendicularDistance(start2, line1) <= collinearThreshold ||
        pointLinePerpendicularDistance(end2, line1) <= collinearThreshold) {
        return true;
    }

    // Additional check: divide line2 into intermediate points and check collinearity
    int numIntermediatePoints = 10;  // You can adjust this based on your needs
    for (int i = 1; i < numIntermediatePoints; ++i) {
        float t = float(i) / float(numIntermediatePoints - 1);    // Calculate a point along line2
        cv::Point2f pointOnLine2 = start2 + t * (end2 - start2);  // Get the point along the line2 segment

        if (pointLinePerpendicularDistance(pointOnLine2, line1) <= collinearThreshold) {
            return true;
        }
    }

    return false;
}

std::vector<cv::Vec4i> combineAlignedLines(std::vector<cv::Vec4i> lines, double angleThreshold, double collinearThreshold) {
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

                    used[j] = true;  // Mark as combined
                    merged = true;   // Signal a merge occurred
                }
            }

            // Ensure the merged line is normalized
            cv::Vec4i combinedLine(start.x, start.y, end.x, end.y);
            normalizeLine(combinedLine);
            newLines.push_back(combinedLine);
        }

        // Update lines with the newly combined lines
        lines = std::move(newLines);

    } while (merged);  // Repeat until no further merges are possible

    return lines;
}

// Function to analyze the combined lines with gyro data and classify them as NORTH, EAST, SOUTH, WEST
std::vector<Direction> analyzeWallDirection(const std::vector<cv::Vec4i>& combinedLines, float gyroYaw, const cv::Point& center) {
    std::vector<Direction> wallDirections;

    // Gyro yaw is assumed to be in degrees with 0° = NORTH, 90° = EAST, 180° = SOUTH, 270° = WEST
    // Adjust the combined line angles based on the gyro data.
    for (const auto& line : combinedLines) {
        double lineAngle = calculateAngle(line);  // Get the angle of the line

        // Determine if the line is more vertical (NORTH/SOUTH) or horizontal (EAST/WEST)
        Direction direction;  // Default direction is NORTH

        bool isLineHorizontal = lineAngle < 45 || lineAngle >= 135;

        Direction gyroDirection = NORTH;
        if (gyroYaw >= 0.0f && gyroYaw < 45.0f) {
            gyroDirection = NORTH;
        } else if (gyroYaw >= 45.0f && gyroYaw < 135.0f) {
            gyroDirection = EAST;
        } else if (gyroYaw >= 135.0f && gyroYaw < 225.0f) {
            gyroDirection = SOUTH;
        } else if (gyroYaw >= 225.0f && gyroYaw < 315.0f) {
            gyroDirection = WEST;
        }

        // Check if the line is in the front, back, left, or right of the robot
        // Using the line's midpoint for classification
        cv::Point midpoint((line[0] + line[2]) / 2, (line[1] + line[3]) / 2);

        if (isLineHorizontal) {
            if (midpoint.y < center.y) {
                switch (gyroDirection) {
                    case NORTH:
                        direction = NORTH;
                        break;
                    case EAST:
                        direction = EAST;
                        break;
                    case SOUTH:
                        direction = SOUTH;
                        break;
                    case WEST:
                        direction = WEST;
                        break;
                }
            } else {
                switch (gyroDirection) {
                    case NORTH:
                        direction = SOUTH;
                        break;
                    case EAST:
                        direction = WEST;
                        break;
                    case SOUTH:
                        direction = NORTH;
                        break;
                    case WEST:
                        direction = EAST;
                        break;
                }
            }
        } else {
            if (midpoint.x > center.x) {
                switch (gyroDirection) {
                    case NORTH:
                        direction = EAST;
                        break;
                    case EAST:
                        direction = SOUTH;
                        break;
                    case SOUTH:
                        direction = WEST;
                        break;
                    case WEST:
                        direction = NORTH;
                        break;
                }
            } else {
                switch (gyroDirection) {
                    case NORTH:
                        direction = WEST;
                        break;
                    case EAST:
                        direction = NORTH;
                        break;
                    case SOUTH:
                        direction = EAST;
                        break;
                    case WEST:
                        direction = SOUTH;
                        break;
                }
            }
        }

        // Add the classified direction to the list
        wallDirections.push_back(direction);
    }

    return wallDirections;
}

float convertLidarDistanceToActualDistance(int scale, double lidarDistance) {
    return lidarDistance / (float)scale;
}
