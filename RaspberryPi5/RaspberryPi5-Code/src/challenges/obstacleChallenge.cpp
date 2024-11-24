#include "obstacleChallenge.h"

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



ObstacleChallenge::ObstacleChallenge(int scale, cv::Point center, float initialGyroYaw) 
    : scale(scale), center(center), initialGyroYaw(initialGyroYaw) {
    // Constructor: Initialize variables or perform setup if needed.
}

float lastUpdateTime = static_cast<float>(cv::getTickCount() / cv::getTickFrequency());
void ObstacleChallenge::update(const cv::Mat& lidarBinaryImage, const cv::Mat& cameraImage, float gyroYaw, float& motorPercent, float& steeringPercent) {
    float currentTime = static_cast<float>(cv::getTickCount()) / cv::getTickFrequency();
    float deltaTime = currentTime - lastUpdateTime;

    motorPercent = 0.40;

    float desiredYaw = directionToHeading(ObstacleChallenge::direction);
    
    // Calculate the relative yaw angle (modulo 360 to normalize)
    float relativeYaw = fmod(gyroYaw - initialGyroYaw + 360.0f, 360.0f);

    // Analyze wall directions using lidar data and relative yaw
    auto lines = detectLines(lidarBinaryImage);
    auto combinedLines = combineAlignedLines(lines);
    auto wallDirections = analyzeWallDirection(combinedLines, relativeYaw, center);

    if (ObstacleChallenge::turnDirection == TurnDirection::UNKNOWN) {
        ObstacleChallenge::turnDirection = lidarDetectTurnDirection(combinedLines, wallDirections, direction);
    }

    float frontWallDistance = NAN;
    float leftWallDistance = NAN;
    float rightWallDistance = NAN;

    cv::Vec4i longestFrontWall;
    cv::Vec4i longestRightWall;
    cv::Vec4i longestLeftWall;

    std::vector<cv::Vec4i> frontWalls;
    std::vector<cv::Vec4i> leftWalls;
    std::vector<cv::Vec4i> rightWalls;
    
    for (size_t i = 0; i < combinedLines.size(); ++i) {
        cv::Vec4i line = combinedLines[i];
        Direction direction = wallDirections[i];  // Get the direction of the current line

        if (direction == ObstacleChallenge::direction) {
            frontWalls.push_back(line);
            continue;
        } else if (direction == calculateRelativeDirection(ObstacleChallenge::direction, RIGHT)) {
            rightWalls.push_back(line);
            continue;
        } else if (direction == calculateRelativeDirection(ObstacleChallenge::direction, LEFT)) {
            leftWalls.push_back(line);
            continue;
        }
    }

    if (!frontWalls.empty()) {
        longestFrontWall = findLongestLine(frontWalls);
        frontWallDistance = convertLidarDistanceToActualDistance(ObstacleChallenge::scale, pointLinePerpendicularDistance(ObstacleChallenge::center, longestFrontWall));
    }

    if (!rightWalls.empty()) {
        longestRightWall = findLongestLine(rightWalls);
        rightWallDistance = convertLidarDistanceToActualDistance(ObstacleChallenge::scale, pointLinePerpendicularDistance(ObstacleChallenge::center, longestRightWall));
    }

    if (!leftWalls.empty()) {
        longestLeftWall = findLongestLine(leftWalls);
        leftWallDistance = convertLidarDistanceToActualDistance(ObstacleChallenge::scale, pointLinePerpendicularDistance(ObstacleChallenge::center, longestLeftWall));
    }

    if (not std::isnan(frontWallDistance)) {
        if (frontWallDistance <= FRONT_WALL_DISTANCE_STOP_THRESHOLD && currentTime - lastTurnTime >= STOP_COOLDOWN && ObstacleChallenge::numberofTurn >= 3*4) {
            ObstacleChallenge::isRunning = false;
        }
        if (frontWallDistance <= FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
            motorPercent = 0.30;
        }
        if (frontWallDistance <= FRONT_WALL_DISTANCE_TURN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
            if (turnDirection == CLOCKWISE) {
                ObstacleChallenge::direction = calculateRelativeDirection(ObstacleChallenge::direction, RIGHT);
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                ObstacleChallenge::direction = calculateRelativeDirection(ObstacleChallenge::direction, LEFT);
            }

            toLeftWallDistance = 0.500;
            lastTurnTime = currentTime;
            ObstacleChallenge::numberofTurn++;
        }
    }

    auto trafficLightPoints = detectTrafficLight(lidarBinaryImage, combinedLines, wallDirections, turnDirection, direction);


    auto cameraImageData = processImage(cameraImage);

    std::vector<BlockInfo> blockAngles;
    for (Block block : cameraImageData.blocks) {
        BlockInfo blockAngle;
        blockAngle.angle = pixelToAngle(block.x, cameraImage.cols, 20, 88.0f);
        blockAngle.size = block.size;
        blockAngle.color = block.color;
        blockAngles.push_back(blockAngle);
        
        // cv::Scalar color;
        // if (blockAngle.color == RED) {
        //     color = cv::Scalar(0, 0, 255);
        // } else {
        //     color = cv::Scalar(0, 255, 0);
        // }

        // drawRadialLines(lidarOutputImage, CENTER, blockAngle.angle, 800, color, 2);
    }
    auto processedTrafficLights = processTrafficLight(trafficLightPoints, blockAngles, center);

    ProcessedBlock closestTrafficLight;
    float closestTrafficLightDistance = std::numeric_limits<float>::max();
    for (auto processedTrafficLight : processedTrafficLights) {
        float distance = cv::norm(processedTrafficLight.point - center);
        if (distance < closestTrafficLightDistance) {
            closestTrafficLightDistance = distance;
            closestTrafficLight = processedTrafficLight;
        }
    }

    float blockDistanceFromLeft = -1; // Assuming the gap between wall is 1 meter
    if (not processedTrafficLights.empty()) {
        if (turnDirection == CLOCKWISE) {
            if (not leftWalls.empty()) {
                blockDistanceFromLeft = pointLinePerpendicularDistance(closestTrafficLight.point, longestLeftWall);
            } else if (not rightWalls.empty()) {
                blockDistanceFromLeft = 1 - pointLinePerpendicularDistance(closestTrafficLight.point, longestRightWall);
            }
        } else {
            if (not rightWalls.empty()) {
                blockDistanceFromLeft = 1 - pointLinePerpendicularDistance(closestTrafficLight.point, longestRightWall);
            } else if (not leftWalls.empty()) {
                blockDistanceFromLeft = pointLinePerpendicularDistance(closestTrafficLight.point, longestLeftWall);
            } 
        }
    }
    printf("blockDistanceFromLeft: %.2f\n", blockDistanceFromLeft);

    if (blockDistanceFromLeft > 0.20*180 and blockDistanceFromLeft < 0.50*180) {
        toLeftWallDistance = 0.220;
    } else if (blockDistanceFromLeft > 0.50*180 and blockDistanceFromLeft < 0.70*180) {
        toLeftWallDistance = 0.750;
    }



    float wallDistanceError;
    if (turnDirection == CLOCKWISE) {
        if (not std::isnan(leftWallDistance)) {
            wallDistanceError = toLeftWallDistance - leftWallDistance;
        } else {
            wallDistanceError = 0;
        }
    } else if (turnDirection == COUNTER_CLOCKWISE) {
        if (not std::isnan(rightWallDistance)) {
            wallDistanceError = -((1 - toLeftWallDistance) - rightWallDistance);
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

    if (not ObstacleChallenge::isRunning) {
        motorPercent = 0.0f;
    } 
    steeringPercent = steeringPID.calculate(fmod(desiredYaw - relativeYaw + 360.0f + 180.0f, 360.0f) - 180.0f, deltaTime);

    // printf("SteeringPercent: %.3f, relativeYaw: %.3f, frontWallDistance: %.3f, leftWallDistance: %.3f, deltaTime: %.3f\n", steeringPercent, relativeYaw, frontWallDistance, leftWallDistance, deltaTime);

    lastUpdateTime = currentTime;
}
