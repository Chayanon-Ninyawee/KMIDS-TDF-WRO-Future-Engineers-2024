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



OpenChallenge::OpenChallenge(int lidarScale, cv::Point lidarCenter) 
    : lidarScale(lidarScale), lidarCenter(lidarCenter) {
    // Constructor: Initialize variables or perform setup if needed.
}

float lastUpdateTime = static_cast<float>(cv::getTickCount() / cv::getTickFrequency());
void OpenChallenge::update(const cv::Mat& lidarBinaryImage, float gyroYaw, float& motorPercent, float& steeringPercent) {
float currentTime = static_cast<float>(cv::getTickCount()) / cv::getTickFrequency();
    float deltaTime = currentTime - lastUpdateTime;

   // Analyze wall directions using lidar data and relative yaw
    auto lines = detectLines(lidarBinaryImage);
    auto combinedLines = combineAlignedLines(lines);
    auto wallDirections = analyzeWallDirection(combinedLines, gyroYaw, lidarCenter);
    auto trafficLightPoints = detectTrafficLight(lidarBinaryImage, combinedLines, wallDirections, turnDirection, robotDirection);

    if (turnDirection == TurnDirection::UNKNOWN) {
        turnDirection = lidarDetectTurnDirection(combinedLines, wallDirections, robotDirection);
    }

    std::vector<cv::Vec4i> frontWalls;
    std::vector<cv::Vec4i> leftWalls;
    std::vector<cv::Vec4i> rightWalls;
    
    for (size_t i = 0; i < combinedLines.size(); ++i) {
        cv::Vec4i line = combinedLines[i];
        Direction direction = wallDirections[i];  // Get the direction of the current line

        // TODO: Fix when the left/right wall is far (it's not inner wall)
        if (direction == robotDirection) {
            frontWalls.push_back(line);
            continue;
        } else if (direction == calculateRelativeDirection(robotDirection, RIGHT)) {
            rightWalls.push_back(line);
            continue;
        } else if (direction == calculateRelativeDirection(robotDirection, LEFT)) {
            leftWalls.push_back(line);
            continue;
        }
    }


    // Select the longest wall to ensure it's the correct one
    cv::Vec4i frontWall;
    cv::Vec4i rightWall;
    cv::Vec4i leftWall;

    float frontWallDistance = NAN;
    float leftWallDistance = NAN;
    float rightWallDistance = NAN;

    if (!frontWalls.empty()) {
        frontWall = findLongestLine(frontWalls);
        frontWallDistance = toMeter(lidarScale, pointLinePerpendicularDistance(lidarCenter, frontWall));
    }

    if (!rightWalls.empty()) {
        rightWall = findLongestLine(rightWalls);
        rightWallDistance = toMeter(lidarScale, pointLinePerpendicularDistance(lidarCenter, rightWall));
    }

    if (!leftWalls.empty()) {
        leftWall = findLongestLine(leftWalls);
        leftWallDistance = toMeter(lidarScale, pointLinePerpendicularDistance(lidarCenter, leftWall));
    }


    switch (state) {
        NORMAL_STATE: {
        case State::NORMAL:
            motorPercent = 1.00;

            if (not std::isnan(frontWallDistance)) {
                if (frontWallDistance <= FRONT_WALL_DISTANCE_STOP_THRESHOLD && currentTime - lastTurnTime >= STOP_COOLDOWN && numberofTurn >= 3 * 4) {
                    state = State::STOP;
                    goto STOP_STATE;
                }
                if (frontWallDistance <= FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
                    motorPercent = 0.50;
                }
                if (frontWallDistance <= FRONT_WALL_DISTANCE_TURN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
                    state = State::TURNING;
                    goto TURNING_STATE;
                }
            }

            float wallDistanceError = 0;
            if (turnDirection == CLOCKWISE) {
                wallDistanceBias = OUTER_WALL_DISTANCE - 0.500;
                if (not std::isnan(leftWallDistance)) {
                    wallDistanceError = wallDistanceBias - (leftWallDistance - 0.500);
                }
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                wallDistanceBias = 0.500 - OUTER_WALL_DISTANCE;
                if (not std::isnan(rightWallDistance)) {
                    wallDistanceError = wallDistanceBias - (0.500 - rightWallDistance);
                }
            } else {
                wallDistanceBias = 0.000;
                if (not std::isnan(leftWallDistance)) {
                    wallDistanceError = wallDistanceBias - (leftWallDistance - 0.500);
                } else if (not std::isnan(rightWallDistance)) {
                    wallDistanceError = wallDistanceBias - (0.500 - rightWallDistance);
                }
            }

            float headingCorrection = wallDistancePID.calculate(wallDistanceError, deltaTime);
            headingCorrection = std::max(std::min(headingCorrection, MAX_HEADING_ERROR), -MAX_HEADING_ERROR);

            float desiredYaw = directionToHeading(robotDirection);
            desiredYaw += headingCorrection;

            float headingError = fmod(desiredYaw - gyroYaw + 360.0f + 180.0f, 360.0f) - 180.0f;
            steeringPercent = steeringPID.calculate(headingError, deltaTime);
            break;
        }
        TURNING_STATE: { // Run once just after changing state
            if (turnDirection == CLOCKWISE) {
                robotDirection = calculateRelativeDirection(robotDirection, RIGHT);
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                robotDirection = calculateRelativeDirection(robotDirection, LEFT);
            }
        case State::TURNING:
            motorPercent = 0.50f;

            float desiredYaw = directionToHeading(robotDirection);
            float headingError = fmod(desiredYaw - gyroYaw + 360.0f + 180.0f, 360.0f) - 180.0f;
            steeringPercent = steeringPID.calculate(headingError, deltaTime);


            if (abs(headingError) < MAX_HEADING_ERROR_BEFORE_EXIT_TURNING) {
                wallDistanceBias = 0.500;
                lastTurnTime = currentTime;
                numberofTurn++;

                state = State::NORMAL;
                goto NORMAL_STATE;
            }
            break;
        }
        STOP_STATE: {
        case State::STOP:
            motorPercent = 0.0f;
            steeringPercent = 0.0f;
            break;
        }
    }

    lastUpdateTime = currentTime;
}