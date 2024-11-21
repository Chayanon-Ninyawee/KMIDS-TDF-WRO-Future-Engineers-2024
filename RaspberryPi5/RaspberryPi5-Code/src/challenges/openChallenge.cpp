#include "openChallenge.h"

// Helper function to calculate line length
float lineLength(const cv::Vec4i& line) {
    return sqrt(pow(line[2] - line[0], 2) + pow(line[3] - line[1], 2));
}

// Find the longest line in a given group of walls
cv::Vec4i findLongestLine(const std::vector<cv::Vec4i>& lines) {
    cv::Vec4i longestLine;
    float maxLength = 0.0f;

    for (const auto& line : lines) {
        float length = lineLength(line);
        if (length > maxLength) {
            maxLength = length;
            longestLine = line;
        }
    }
    return longestLine;
}



OpenChallenge::OpenChallenge(int scale, cv::Point center, float initialGyroYaw) 
    : scale(scale), center(center), initialGyroYaw(initialGyroYaw) {
    // Constructor: Initialize variables or perform setup if needed.
}


void OpenChallenge::update(const std::vector<cv::Vec4i>& combined_lines, float gyroYaw, float& motorPercent, float& steeringPercent) {
    float desiredYaw = directionToHeading(OpenChallenge::direction);
    
    // Calculate the relative yaw angle (modulo 360 to normalize)
    float relativeYaw = fmod(gyroYaw - initialGyroYaw + 360.0f, 360.0f);

    // Analyze wall directions using lidar data and relative yaw
    auto wallDirections = analyzeWallDirection(combined_lines, relativeYaw, center);

    float frontWallDistance = OpenChallenge::frontWallDistanceTurnThreshold + 0.1;
    float innerWallDistance = OpenChallenge::innerWallDistance;

    std::vector<cv::Vec4i> frontWalls;
    std::vector<cv::Vec4i> innerWalls;
    std::vector<cv::Vec4i> outerWalls;
    
    for (size_t i = 0; i < combined_lines.size(); ++i) {
        cv::Vec4i line = combined_lines[i];
        Direction direction = wallDirections[i];  // Get the direction of the current line

        if (direction == OpenChallenge::direction) {
            frontWalls.push_back(line);
            continue;
        }

        if (turnDirection == CLOCKWISE) {
            if (direction == calculateRelativeDirection(OpenChallenge::direction, RIGHT)) {
                innerWalls.push_back(line);
                continue;
            } else if (direction == calculateRelativeDirection(OpenChallenge::direction, LEFT)) {
                outerWalls.push_back(line);
                continue;
            }
        } else if (turnDirection == COUNTER_CLOCKWISE) {
            if (direction == calculateRelativeDirection(OpenChallenge::direction, RIGHT)) {
                outerWalls.push_back(line);
                continue;
            } else if (direction == calculateRelativeDirection(OpenChallenge::direction, LEFT)) {
                innerWalls.push_back(line);
                continue;
            }
        }
    }

    if (!frontWalls.empty()) {
        cv::Vec4i longestFrontWall = findLongestLine(frontWalls);
        frontWallDistance = convertLidarDistanceToActualDistance(OpenChallenge::scale, pointLinePerpendicularDistance(OpenChallenge::center, longestFrontWall));
    }

    if (!innerWalls.empty()) {
        cv::Vec4i longestInnerWall = findLongestLine(innerWalls);
        innerWallDistance = convertLidarDistanceToActualDistance(OpenChallenge::scale, pointLinePerpendicularDistance(OpenChallenge::center, longestInnerWall));
    }

    if (frontWallDistance <= frontWallDistanceTurnThreshold) {
        if (turnDirection == CLOCKWISE) {
            OpenChallenge::direction = calculateRelativeDirection(OpenChallenge::direction, RIGHT);
        } else if (turnDirection == COUNTER_CLOCKWISE) {
            OpenChallenge::direction = calculateRelativeDirection(OpenChallenge::direction, LEFT);
        }
    }

    // Calculate steering correction
    static float lastUpdateTime = static_cast<float>(cv::getTickCount());
    float currentTime = static_cast<float>(cv::getTickCount());
    float deltaTime = (currentTime - lastUpdateTime) / cv::getTickFrequency();
    lastUpdateTime = currentTime;

    motorPercent = 0.7;
    steeringPercent = steeringPID.calculate(fmod(desiredYaw - relativeYaw + 360.0f + 180.0f, 360.0f) - 180.0f, deltaTime);

    printf("SteeringPercent: %.3f\n", steeringPercent);
}