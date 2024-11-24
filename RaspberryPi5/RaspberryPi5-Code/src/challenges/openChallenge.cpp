#include "openChallenge.h"

#include <algorithm>

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

float lastUpdateTime = static_cast<float>(cv::getTickCount() / cv::getTickFrequency());
void OpenChallenge::update(const std::vector<cv::Vec4i>& combined_lines, float gyroYaw, float& motorPercent, float& steeringPercent) {
    float currentTime = static_cast<float>(cv::getTickCount()) / cv::getTickFrequency();
    float deltaTime = currentTime - lastUpdateTime;

    motorPercent = 1.00;

    float desiredYaw = directionToHeading(OpenChallenge::direction);
    
    // Calculate the relative yaw angle (modulo 360 to normalize)
    float relativeYaw = fmod(gyroYaw - initialGyroYaw + 360.0f, 360.0f);

    // Analyze wall directions using lidar data and relative yaw
    auto wallDirections = analyzeWallDirection(combined_lines, relativeYaw, center);

    if (OpenChallenge::turnDirection == TurnDirection::UNKNOWN) {
        OpenChallenge::turnDirection = lidarDetectTurnDirection(combined_lines, wallDirections, direction);
    }

    float frontWallDistance = NAN;
    float leftWallDistance = NAN;
    float rightWallDistance = NAN;

    std::vector<cv::Vec4i> frontWalls;
    std::vector<cv::Vec4i> leftWalls;
    std::vector<cv::Vec4i> rightWalls;
    
    for (size_t i = 0; i < combined_lines.size(); ++i) {
        cv::Vec4i line = combined_lines[i];
        Direction direction = wallDirections[i];  // Get the direction of the current line

        if (direction == OpenChallenge::direction) {
            frontWalls.push_back(line);
            continue;
        } else if (direction == calculateRelativeDirection(OpenChallenge::direction, RIGHT)) {
            rightWalls.push_back(line);
            continue;
        } else if (direction == calculateRelativeDirection(OpenChallenge::direction, LEFT)) {
            leftWalls.push_back(line);
            continue;
        }
    }

    if (!frontWalls.empty()) {
        cv::Vec4i longestFrontWall = findLongestLine(frontWalls);
        frontWallDistance = convertLidarDistanceToActualDistance(OpenChallenge::scale, pointLinePerpendicularDistance(OpenChallenge::center, longestFrontWall));
    }

    if (!rightWalls.empty()) {
        cv::Vec4i longestRightWall = findLongestLine(rightWalls);
        rightWallDistance = convertLidarDistanceToActualDistance(OpenChallenge::scale, pointLinePerpendicularDistance(OpenChallenge::center, longestRightWall));
    }

    if (!leftWalls.empty()) {
        cv::Vec4i longestLeftWall = findLongestLine(leftWalls);
        leftWallDistance = convertLidarDistanceToActualDistance(OpenChallenge::scale, pointLinePerpendicularDistance(OpenChallenge::center, longestLeftWall));
    }

    if (not std::isnan(frontWallDistance)) {
        if (frontWallDistance <= FRONT_WALL_DISTANCE_STOP_THRESHOLD && currentTime - lastTurnTime >= STOP_COOLDOWN && OpenChallenge::numberofTurn >= 3*4) {
            OpenChallenge::isRunning = false;
        }
        if (frontWallDistance <= FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
            motorPercent = 0.50;
        }
        if (frontWallDistance <= FRONT_WALL_DISTANCE_TURN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
            if (turnDirection == CLOCKWISE) {
                OpenChallenge::direction = calculateRelativeDirection(OpenChallenge::direction, RIGHT);
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                OpenChallenge::direction = calculateRelativeDirection(OpenChallenge::direction, LEFT);
            }

            lastTurnTime = currentTime;
            OpenChallenge::numberofTurn++;
        }
    }

    float wallDistanceError;
    if (turnDirection == CLOCKWISE) {
        if (not std::isnan(leftWallDistance)) {
            wallDistanceError = OUTER_WALL_DISTANCE - leftWallDistance;
        } else {
            wallDistanceError = 0;
        }
    } else if (turnDirection == COUNTER_CLOCKWISE) {
        if (not std::isnan(rightWallDistance)) {
            wallDistanceError = -(OUTER_WALL_DISTANCE - rightWallDistance);
        } else {
            wallDistanceError = 0;
        }
    } else {
        if ((not std::isnan(leftWallDistance)) && (not std::isnan(rightWallDistance))) {
            wallDistanceError = (leftWallDistance - rightWallDistance) / 2.0f;
        } else {
            wallDistanceError = 0;
        }
    }

    float headingCorrection = wallDistancePID.calculate(wallDistanceError, deltaTime);
    headingCorrection = std::max(std::min(headingCorrection, MAX_HEADING_ERROR), -MAX_HEADING_ERROR);

    desiredYaw += headingCorrection;

    if (not OpenChallenge::isRunning) {
        motorPercent = 0.0f;
    } 
    steeringPercent = steeringPID.calculate(fmod(desiredYaw - relativeYaw + 360.0f + 180.0f, 360.0f) - 180.0f, deltaTime);

    printf("SteeringPercent: %.3f, relativeYaw: %.3f, frontWallDistance: %.3f, leftWallDistance: %.3f, deltaTime: %.3f\n", steeringPercent, relativeYaw, frontWallDistance, leftWallDistance, deltaTime);

    lastUpdateTime = currentTime;
}
