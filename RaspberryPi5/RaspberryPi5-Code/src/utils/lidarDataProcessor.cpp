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

float toMeter(int scale, double lidarDistance) {
    return lidarDistance / (float)scale;
}

// Detect lines using Hough Transform
std::vector<cv::Vec4i> detectLines(const cv::Mat& binaryImage) {
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(binaryImage, lines, 1, CV_PI / 180, 20, 60, 30);
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

double pointLinePerpendicularDirection(const cv::Point2f& pt, const cv::Vec4i& line) {
    // Extract line endpoints
    cv::Point2f lineStart(line[0], line[1]);
    cv::Point2f lineEnd(line[2], line[3]);

    // Compute the direction vector of the line
    cv::Point2f lineVec = lineEnd - lineStart;

    // Compute the perpendicular vector to the line
    cv::Point2f perpVec(-lineVec.y, lineVec.x); // Rotate by 90 degrees counterclockwise

    // Compute the vector from the line to the point
    cv::Point2f pointVec = pt - lineStart;

    // Determine the relative direction (dot product)
    double dot = pointVec.x * perpVec.x + pointVec.y * perpVec.y;

    // Determine the angle of the perpendicular vector (atan2 returns in radians)
    double angle = std::atan2(perpVec.y, perpVec.x) * 180.0 / CV_PI;

    // Normalize the angle to 0–360 degrees
    if (dot < 0) {
        angle += 180.0; // Flip direction if the point is on the other side
    }
    angle = std::fmod(angle + 360.0, 360.0);

    return angle;
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
        // double lineAngle = calculateAngle(line);
        double perpendicularLineDirection = pointLinePerpendicularDirection(center, line);

        // double relativeLineAngle = fmod(lineAngle + gyroYaw + 360.0f, 360.0f);
        double relativePerpendicularLineDirection = fmod(perpendicularLineDirection + gyroYaw - 90.0f + 360.0f, 360.0f);

        Direction direction = NORTH;
        if (relativePerpendicularLineDirection >= 315 || relativePerpendicularLineDirection < 45) direction = NORTH;
        else if (relativePerpendicularLineDirection >= 45 && relativePerpendicularLineDirection < 135) direction = EAST;
        else if (relativePerpendicularLineDirection >= 135 && relativePerpendicularLineDirection < 225) direction = SOUTH;
        else if (relativePerpendicularLineDirection >= 225 && relativePerpendicularLineDirection < 315) direction = WEST;

        // Add the classified direction to the list
        wallDirections.push_back(direction);
    }

    return wallDirections;
}

std::vector<cv::Point> detectTrafficLight(const cv::Mat& binaryImage, const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, TurnDirection turnDirection, Direction direction) {
    cv::Mat dilatedBinaryImage = binaryImage.clone();
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(20, 20));
    cv::dilate(binaryImage, dilatedBinaryImage, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(dilatedBinaryImage, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Point> trafficLightPoints;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);

        if (area >= 300 && area <= 1400) {
            cv::Moments moments = cv::moments(contour);

            int centroidX = static_cast<int>(moments.m10 / moments.m00);
            int centroidY = static_cast<int>(moments.m01 / moments.m00);

            cv::Point point(centroidX, centroidY);

            cv::Vec4i frontLine(-1.0f, -1.0f, -1.0f, -1.0f); // Initialize with a sentinel value (-1, -1, -1, -1)
            std::vector<cv::Vec4i> leftLines;
            std::vector<cv::Vec4i> rightLines;

            for (size_t i = 0; i < combinedLines.size(); ++i) {
                cv::Vec4i line = combinedLines[i];
                Direction wallDirection = wallDirections[i];

                if (wallDirection == calculateRelativeDirection(direction, FRONT)) {
                    if (not (frontLine == cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f))) {
                        cv::Point2f start(line[0], line[1]);
                        cv::Point2f end(line[2], line[3]);
                        cv::Point2f newFrontMidPoint = cv::Point2f((start.x + end.x) / 2, (start.y + end.y) / 2);

                        cv::Point2f frontStart(frontLine[0], frontLine[1]);
                        cv::Point2f frontEnd(frontLine[2], frontLine[3]);
                        cv::Point2f frontMidPoint = cv::Point2f((frontStart.x + frontEnd.x) / 2, (frontStart.y + frontEnd.y) / 2);

                        if (frontMidPoint.y > newFrontMidPoint.y) { // Check if new midpoint is higher
                            frontLine = line;
                        }
                    } else {
                        frontLine = line;
                    }
                } else if (wallDirection == calculateRelativeDirection(direction, LEFT)) {
                    leftLines.push_back(line);
                } else if (wallDirection == calculateRelativeDirection(direction, RIGHT)) {
                    rightLines.push_back(line);
                }
            }


            double frontDistance = 0;
            double outerDistance = 0;
            double outerFarDistance = -1; // Initialize with a sentinel value -1

            if (frontLine != cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f)) frontDistance = pointLinePerpendicularDistance(point, frontLine);
            
            if (turnDirection == CLOCKWISE) {
                if (not leftLines.empty()) {
                    outerDistance = pointLinePerpendicularDistance(point, leftLines[0]);
                }
                if ((not rightLines.empty()) && (frontLine != cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f))) {
                    cv::Point2f frontStart(frontLine[0], frontLine[1]);
                    cv::Point2f frontEnd(frontLine[2], frontLine[3]);

                    cv::Point2f frontLefter;
                    cv::Point2f frontRighter;

                    if (frontStart.x < frontEnd.x) {
                        frontLefter = frontStart;
                        frontRighter = frontEnd;
                    } else {
                        frontLefter = frontEnd;
                        frontRighter = frontStart;
                    }

                    for (auto rightLine : rightLines) {
                        cv::Point2f rightStart(rightLine[0], rightLine[1]);
                        cv::Point2f rightEnd(rightLine[2], rightLine[3]);

                        cv::Point2f rightHigher;
                        if (rightStart.y < rightEnd.y) {
                            rightHigher = rightStart;
                        } else {
                            rightHigher = rightEnd;
                        }

                        if (rightHigher.x - 30 /* TODO: Remove this magic nubmer */ > frontRighter.x) {
                            if (rightHigher.x > 1200 - 120 /* TODO: Remove this magic nubmer */) {
                                outerFarDistance = pointLinePerpendicularDistance(point, rightLine);
                            }
                        }
                    }
                }
            } else if (turnDirection == COUNTER_CLOCKWISE || turnDirection == UNKNOWN) {
                if (not rightLines.empty()) {
                    outerDistance = pointLinePerpendicularDistance(point, rightLines[0]);
                }
                if ((not leftLines.empty()) && (frontLine != cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f))) {
                    cv::Point2f frontStart(frontLine[0], frontLine[1]);
                    cv::Point2f frontEnd(frontLine[2], frontLine[3]);

                    cv::Point2f frontLefter;
                    cv::Point2f frontRighter;

                    if (frontStart.x < frontEnd.x) {
                        frontLefter = frontStart;
                        frontRighter = frontEnd;
                    } else {
                        frontLefter = frontEnd;
                        frontRighter = frontStart;
                    }

                    for (auto leftLine : leftLines) {
                        cv::Point2f leftStart(leftLine[0], leftLine[1]);
                        cv::Point2f leftEnd(leftLine[2], leftLine[3]);

                        cv::Point2f leftHigher;
                        if (leftStart.y < leftEnd.y) {
                            leftHigher = leftStart;
                        } else {
                            leftHigher = leftEnd;
                        }

                        if (leftHigher.x + 30 /* TODO: Remove this magic nubmer */ < frontLefter.x) {
                            if (leftHigher.x < 120 /* TODO: Remove this magic nubmer */) {
                                outerFarDistance = pointLinePerpendicularDistance(point, leftLine);
                            }
                        }
                    }
                }
            }

            // Put these thing in the header files
            const double outerEdge = 40;
            const double innerEdge = 140;

            if (outerFarDistance != -1) {
                if (frontDistance < outerEdge or outerDistance < outerEdge or outerFarDistance < outerEdge) continue;
                if (frontDistance > innerEdge and outerDistance > innerEdge and outerFarDistance > innerEdge) continue;
            } else {
                if (frontDistance < outerEdge or outerDistance < outerEdge) continue;
                if (frontDistance > innerEdge and outerDistance > innerEdge) continue;
            }

            trafficLightPoints.push_back(point);
        }
    }

    return trafficLightPoints;
}

std::vector<cv::Vec4i> detectParkingZone(const cv::Mat& binaryImage, const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, TurnDirection turnDirection, Direction direction) {
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(binaryImage, lines, 1, CV_PI / 180, 20, 30, 30);

    // Filter lines based on length and proximity to combinedLines
    const double MAX_LENGTH = 60.0;
    const double MAX_DISTANCE = 10.0; // Distance threshold for proximity
    std::vector<cv::Vec4i> filteredLines;

    for (const auto& line : lines) {
        // Calculate the length of the line
        double length = std::sqrt(std::pow(line[2] - line[0], 2) + std::pow(line[3] - line[1], 2));
        if (length > MAX_LENGTH) continue;

        // Check proximity of both endpoints to the combined lines
        cv::Point p1(line[0], line[1]);
        cv::Point p2(line[2], line[3]);

        cv::Vec4i frontLine(-1.0f, -1.0f, -1.0f, -1.0f);  // Initialize with a sentinel value (-1, -1, -1, -1)
        std::vector<cv::Vec4i> leftLines;
        std::vector<cv::Vec4i> rightLines;

        for (size_t i = 0; i < combinedLines.size(); ++i) {
            cv::Vec4i line = combinedLines[i];
            Direction wallDirection = wallDirections[i];

            if (wallDirection == calculateRelativeDirection(direction, FRONT)) {
                if (not(frontLine == cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f))) {
                    cv::Point2f start(line[0], line[1]);
                    cv::Point2f end(line[2], line[3]);
                    cv::Point2f newFrontMidPoint = cv::Point2f((start.x + end.x) / 2, (start.y + end.y) / 2);

                    cv::Point2f frontStart(frontLine[0], frontLine[1]);
                    cv::Point2f frontEnd(frontLine[2], frontLine[3]);
                    cv::Point2f frontMidPoint = cv::Point2f((frontStart.x + frontEnd.x) / 2, (frontStart.y + frontEnd.y) / 2);

                    if (frontMidPoint.y > newFrontMidPoint.y) {  // Check if new midpoint is higher
                        frontLine = line;
                    }
                } else {
                    frontLine = line;
                }
            } else if (wallDirection == calculateRelativeDirection(direction, LEFT)) {
                leftLines.push_back(line);
            } else if (wallDirection == calculateRelativeDirection(direction, RIGHT)) {
                rightLines.push_back(line);
            }
        }

        double frontDistance1 = 0;
        double frontDistance2 = 0;
        double outerDistance1 = 0;
        double outerDistance2 = 0;
        double outerFarDistance1 = -1;  // Initialize with a sentinel value -1
        double outerFarDistance2 = -1;  // Initialize with a sentinel value -1

        if (frontLine != cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f)) {
            frontDistance1 = pointLinePerpendicularDistance(p1, frontLine);
            frontDistance2 = pointLinePerpendicularDistance(p2, frontLine);
        }

        if (turnDirection == CLOCKWISE) {
            if (not leftLines.empty()) {
                outerDistance1 = pointLinePerpendicularDistance(p1, leftLines[0]);
                outerDistance2 = pointLinePerpendicularDistance(p2, leftLines[0]);
            }
            if ((not rightLines.empty()) && (frontLine != cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f))) {
                cv::Point2f frontStart(frontLine[0], frontLine[1]);
                cv::Point2f frontEnd(frontLine[2], frontLine[3]);

                cv::Point2f frontLefter;
                cv::Point2f frontRighter;

                if (frontStart.x < frontEnd.x) {
                    frontLefter = frontStart;
                    frontRighter = frontEnd;
                } else {
                    frontLefter = frontEnd;
                    frontRighter = frontStart;
                }

                for (auto rightLine : rightLines) {
                    cv::Point2f rightStart(rightLine[0], rightLine[1]);
                    cv::Point2f rightEnd(rightLine[2], rightLine[3]);

                    cv::Point2f rightHigher;
                    if (rightStart.y < rightEnd.y) {
                        rightHigher = rightStart;
                    } else {
                        rightHigher = rightEnd;
                    }

                    if (rightHigher.x - 30 /* TODO: Remove this magic nubmer */ > frontRighter.x) {
                        if (rightHigher.x > 1200 - 120 /* TODO: Remove this magic nubmer */) {
                            outerFarDistance1 = pointLinePerpendicularDistance(p1, rightLine);
                            outerFarDistance2 = pointLinePerpendicularDistance(p2, rightLine);
                        }
                    }
                }
            }
        } else if (turnDirection == COUNTER_CLOCKWISE || turnDirection == UNKNOWN) {
            if (not rightLines.empty()) {
                outerDistance1 = pointLinePerpendicularDistance(p1, rightLines[0]);
                outerDistance2 = pointLinePerpendicularDistance(p2, rightLines[0]);
            }
            if ((not leftLines.empty()) && (frontLine != cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f))) {
                cv::Point2f frontStart(frontLine[0], frontLine[1]);
                cv::Point2f frontEnd(frontLine[2], frontLine[3]);

                cv::Point2f frontLefter;
                cv::Point2f frontRighter;

                if (frontStart.x < frontEnd.x) {
                    frontLefter = frontStart;
                    frontRighter = frontEnd;
                } else {
                    frontLefter = frontEnd;
                    frontRighter = frontStart;
                }

                for (auto leftLine : leftLines) {
                    cv::Point2f leftStart(leftLine[0], leftLine[1]);
                    cv::Point2f leftEnd(leftLine[2], leftLine[3]);

                    cv::Point2f leftHigher;
                    if (leftStart.y < leftEnd.y) {
                        leftHigher = leftStart;
                    } else {
                        leftHigher = leftEnd;
                    }

                    if (leftHigher.x + 30 /* TODO: Remove this magic nubmer */ < frontLefter.x) {
                        if (leftHigher.x < 120 /* TODO: Remove this magic nubmer */) {
                            outerFarDistance1 = pointLinePerpendicularDistance(p1, leftLine);
                            outerFarDistance2 = pointLinePerpendicularDistance(p2, leftLine);
                        }
                    }
                }
            }
        }

        // Put these thing in the header files
        const double frontOuterEdge = 40;
        const double outerEdge = 20;
        const double innerEdge = 160;

        bool isDistanceClose1 = false;
        bool isDistanceClose2 = false;

        if (outerFarDistance1 != -1) {
            if (frontDistance1 < frontOuterEdge or frontDistance1 > 540 - frontOuterEdge or outerDistance1 < outerEdge or outerFarDistance1 < outerEdge) isDistanceClose1 = true;
            if (frontDistance1 > innerEdge and outerDistance1 > innerEdge and outerFarDistance1 > innerEdge) isDistanceClose1 = true;
        } else {
            if (frontDistance1 < frontOuterEdge or frontDistance1 > 540 - frontOuterEdge or outerDistance1 < outerEdge) isDistanceClose1 = true;
            if (frontDistance1 > innerEdge and outerDistance1 > innerEdge) isDistanceClose1 = true;
        }

        if (outerFarDistance2 != -1) {
            if (frontDistance2 < frontOuterEdge or frontDistance2 > 540 - frontOuterEdge or outerDistance2 < outerEdge or outerFarDistance2 < outerEdge) isDistanceClose2 = true;
            if (frontDistance2 > innerEdge and outerDistance2 > innerEdge and outerFarDistance2 > innerEdge) isDistanceClose2 = true;
        } else {
            if (frontDistance2 < frontOuterEdge or frontDistance2 > 540 - frontOuterEdge or outerDistance2 < outerEdge) isDistanceClose2 = true;
            if (frontDistance2 > innerEdge and outerDistance2 > innerEdge) isDistanceClose2 = true;
        }

        if (not (isDistanceClose1 && isDistanceClose2)) {
            filteredLines.push_back(line);
        }
    }

    return filteredLines;
}

std::vector<ProcessedTrafficLight> processTrafficLight(
    const std::vector<cv::Point>& trafficLightPoints, 
    const std::vector<BlockInfo>& blockInfos,
    const cv::Point& center
) {
    constexpr int BLOCK_SIZE_TO_PRIORITIZE_DISTANCE = 3000;
    constexpr int MIN_BLOCK_SIZE = 0;
    constexpr float MAX_LIDAR_CAM_ANGLE_DIFFERENCE = 8.0f;

    std::vector<ProcessedTrafficLight> processedBlocks;

    for (const BlockInfo& blockInfo : blockInfos) {
        if (blockInfo.size < MIN_BLOCK_SIZE) continue;

        ProcessedTrafficLight processedBlock;
        processedBlock.color = blockInfo.color;
        processedBlock.size = blockInfo.size;

        float correspondingAngle = blockInfo.angle;

        cv::Point closestPoint;
        float minAngleDifference = std::numeric_limits<float>::max();
        float minDistance = std::numeric_limits<float>::max();

        for (const cv::Point& point : trafficLightPoints) {
            // Calculate the angle between the center and the traffic light point in radians
            float angleRadians = std::atan2(-(point.y - center.y), point.x - center.x);
            float angleDegrees = angleRadians * (180 / M_PI);  // Convert to degrees
            angleDegrees = fmod(90.0f - angleDegrees + 720.0f, 360.0f);

            if (not ((angleDegrees > 320 && angleDegrees <= 360) || (angleDegrees >= 0 && angleDegrees < 40))) continue; // TODO: Remove this magic numbers

            // Calculate the absolute difference between angles
            float angleDifference = std::fmod(std::abs(angleDegrees - correspondingAngle), 360.0f);
            angleDifference = std::min(angleDifference, 360.0f - angleDifference);

            float distance = cv::norm(point - center);

            if (angleDifference > MAX_LIDAR_CAM_ANGLE_DIFFERENCE) continue;

            if (blockInfo.size >= BLOCK_SIZE_TO_PRIORITIZE_DISTANCE) {
                if (distance < minDistance) {
                    minAngleDifference = angleDifference;
                    minDistance = distance;
                    closestPoint = point;
                }
            } else if (angleDifference < minAngleDifference) {
                minAngleDifference = angleDifference;
                closestPoint = point;
            }
        }

        if (minAngleDifference == std::numeric_limits<float>::max()) continue;

        // Assign the selected point
        processedBlock.point = closestPoint;
        processedBlocks.push_back(processedBlock);
    }

    return processedBlocks;
}


TurnDirection lidarDetectTurnDirection(const std::vector<cv::Vec4i>& combinedLines, const std::vector<Direction>& wallDirections, Direction direction) {
    cv::Vec4i frontLine(-1.0f, -1.0f, -1.0f, -1.0f); // Initialize with a sentinel value (-1, -1, -1, -1)
    std::vector<cv::Vec4i> leftLines;
    std::vector<cv::Vec4i> rightLines;

    for (size_t i = 0; i < combinedLines.size(); ++i) {
        cv::Vec4i line = combinedLines[i];
        Direction wallDirection = wallDirections[i];

        if (wallDirection == calculateRelativeDirection(direction, FRONT)) {
            if (not (frontLine == cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f))) {
                cv::Point2f start(line[0], line[1]);
                cv::Point2f end(line[2], line[3]);
                cv::Point2f newFrontMidPoint = cv::Point2f((start.x + end.x) / 2, (start.y + end.y) / 2);

                cv::Point2f frontStart(frontLine[0], frontLine[1]);
                cv::Point2f frontEnd(frontLine[2], frontLine[3]);
                cv::Point2f frontMidPoint = cv::Point2f((frontStart.x + frontEnd.x) / 2, (frontStart.y + frontEnd.y) / 2);

                if (frontMidPoint.y > newFrontMidPoint.y) { // Check if new midpoint is higher
                    frontLine = line;
                }
            } else {
                frontLine = line;
            }
        } else if (wallDirection == calculateRelativeDirection(direction, LEFT)) {
            leftLines.push_back(line);
        } else if (wallDirection == calculateRelativeDirection(direction, RIGHT)) {
            rightLines.push_back(line);
        }
    }

    if (frontLine == cv::Vec4i(-1.0f, -1.0f, -1.0f, -1.0f)) return TurnDirection::UNKNOWN;
    if (leftLines.empty() && rightLines.empty()) return TurnDirection::UNKNOWN;

    cv::Point2f frontStart(frontLine[0], frontLine[1]);
    cv::Point2f frontEnd(frontLine[2], frontLine[3]);
    // cv::Point2f frontMidPoint = cv::Point2f((frontStart.x + frontEnd.x) / 2, (frontStart.y + frontEnd.y) / 2);

    cv::Point2f frontLefter;
    cv::Point2f frontRighter;
    if (frontStart.x < frontEnd.x) {
        frontLefter = frontStart;
        frontRighter = frontEnd;
    } else {
        frontLefter = frontEnd;
        frontRighter = frontStart;
    }

    for (auto leftLine : leftLines) {
        cv::Point2f leftStart(leftLine[0], leftLine[1]);
        cv::Point2f leftEnd(leftLine[2], leftLine[3]);

        cv::Point2f leftHigher;
        if (leftStart.y < leftEnd.y) {
            leftHigher = leftStart;
        } else {
            leftHigher = leftEnd;
        }

        if (leftHigher.x + 30 /* TODO: Remove this magic nubmer */ < frontLefter.x) {
            if (leftHigher.x < 120 /* TODO: Remove this magic nubmer */) return TurnDirection::COUNTER_CLOCKWISE;
            continue;
        }

        if (abs(frontLefter.y - leftHigher.y) < 60) return TurnDirection::CLOCKWISE;
        if (leftHigher.x - 40 /* TODO: Remove this magic nubmer */ > frontLefter.x) return TurnDirection::COUNTER_CLOCKWISE;
    }

    for (auto rightLine : rightLines) {
        cv::Point2f rightStart(rightLine[0], rightLine[1]);
        cv::Point2f rightEnd(rightLine[2], rightLine[3]);

        cv::Point2f rightHigher;
        if (rightStart.y < rightEnd.y) {
            rightHigher = rightStart;
        } else {
            rightHigher = rightEnd;
        }

        if (rightHigher.x - 30 /* TODO: Remove this magic nubmer */ > frontRighter.x) {
            if (rightHigher.x > 1200 - 120 /* TODO: Remove this magic nubmer */) return TurnDirection::CLOCKWISE;
            continue;
        }

        if (abs(frontRighter.y - rightHigher.y) < 60 /* TODO: Remove this magic nubmer */) return TurnDirection::COUNTER_CLOCKWISE;
        if (rightHigher.x + 40 /* TODO: Remove this magic nubmer */ < frontRighter.x) return TurnDirection::CLOCKWISE;
    }

    return TurnDirection::UNKNOWN;
}


void drawAllLines(cv::Mat &outputImage, const std::vector<cv::Vec4i> &lines, const std::vector<Direction> &wallDirections) {
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

void drawTrafficLights(cv::Mat& outputImage, const std::vector<ProcessedTrafficLight>& processedBlocks) {
    // Iterate through the processed blocks and draw the traffic light points
    for (const ProcessedTrafficLight& block : processedBlocks) {
        // Draw the point (circle) at the traffic light location
        cv::Scalar color;
        switch (block.color) {
            case RED:    color = cv::Scalar(0, 0, 255); break;   // Red
            case GREEN:  color = cv::Scalar(0, 255, 0); break;   // Green
        }

        // Draw a circle at the traffic light's position
        cv::circle(outputImage, block.point, 10, color, -1);  // Filled circle with radius 10
    }
}