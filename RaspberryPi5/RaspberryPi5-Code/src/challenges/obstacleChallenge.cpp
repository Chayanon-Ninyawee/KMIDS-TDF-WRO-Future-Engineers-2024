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

ObstacleChallenge::ObstacleChallenge(int lidarScale, cv::Point lidarCenter)
    : lidarScale(lidarScale), lidarCenter(lidarCenter) {
    // Constructor: Initialize variables or perform setup if needed.
}

float lastUpdateTime = static_cast<float>(cv::getTickCount() / cv::getTickFrequency());
void ObstacleChallenge::update(const cv::Mat& lidarBinaryImage, const cv::Mat& cameraImage, float gyroYaw, float& motorPercent, float& steeringPercent) {
    float currentTime = static_cast<float>(cv::getTickCount()) / cv::getTickFrequency();
    float deltaTime = currentTime - lastUpdateTime;

    /*
    TODO
    - UTURN
    - PARKING
    */

    // Analyze wall directions using lidar data and relative yaw
    auto lines = detectLines(lidarBinaryImage);
    auto combinedLines = combineAlignedLines(lines);
    auto wallDirections = analyzeWallDirection(combinedLines, gyroYaw, lidarCenter);
    auto trafficLightPoints = detectTrafficLight(lidarBinaryImage, combinedLines, wallDirections, turnDirection, robotDirection);

    auto cameraImageData = processImage(cameraImage);

    std::vector<BlockInfo> blockAngles;
    for (Block block : cameraImageData.blocks) {
        BlockInfo blockAngle;
        blockAngle.angle = pixelToAngle(block.x, cameraImage.cols, 20, 88.0f);
        blockAngle.size = block.size;
        blockAngle.color = block.color;
        blockAngles.push_back(blockAngle);
    }
    auto processedTrafficLights = processTrafficLight(trafficLightPoints, blockAngles, lidarCenter);

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

    if (!isnan(frontWallDistance)) {
        if (turnDirection == TurnDirection::CLOCKWISE) {
            if (!isnan(leftWallDistance)) {
                for (auto processedTrafficLight : processedTrafficLights) {
                    if (processedTrafficLight.size < 600) continue;

                    float trafficLightDistanceFromLeft = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, leftWall));
                    float trafficLightDistanceFromFront = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, frontWall));

                    TrafficLightPosition trafficLightPosition;
                    TrafficLightRingPosition trafficLightRingPosition;
                    Direction relativeDirection;

                    if (trafficLightDistanceFromLeft < 0.900) {
                        // Determine TrafficLightPosition based on distance from the front
                        if (trafficLightDistanceFromFront > 0.80 && trafficLightDistanceFromFront < 1.20) {
                            trafficLightPosition = TrafficLightPosition::ORANGE;
                        } else if (trafficLightDistanceFromFront > 1.30 && trafficLightDistanceFromFront < 1.70) {
                            trafficLightPosition = TrafficLightPosition::MID;
                        } else if (trafficLightDistanceFromFront > 1.80 && trafficLightDistanceFromFront < 2.20) {
                            trafficLightPosition = TrafficLightPosition::BLUE;
                        }

                        // Determine TrafficLightRingPosition
                        if (trafficLightDistanceFromLeft < 0.500) {
                            trafficLightRingPosition = TrafficLightRingPosition::OUTER;
                        } else {
                            trafficLightRingPosition = TrafficLightRingPosition::INNER;
                        }

                        relativeDirection = calculateRelativeDirection(robotDirection, LEFT);
                    } else {
                        // Determine TrafficLightPosition based on distance from the left
                        if (trafficLightDistanceFromLeft >= 0.900 && trafficLightDistanceFromLeft < 1.20) {
                            trafficLightPosition = TrafficLightPosition::BLUE;
                        } else if (trafficLightDistanceFromLeft > 1.30 && trafficLightDistanceFromLeft < 1.70) {
                            trafficLightPosition = TrafficLightPosition::MID;
                        } else if (trafficLightDistanceFromLeft > 1.80 && trafficLightDistanceFromLeft < 2.20) {
                            trafficLightPosition = TrafficLightPosition::ORANGE;
                        }

                        // Determine TrafficLightRingPosition
                        if (trafficLightDistanceFromFront < 0.500) {
                            trafficLightRingPosition = TrafficLightRingPosition::OUTER;
                        } else {
                            trafficLightRingPosition = TrafficLightRingPosition::INNER;
                        }

                        relativeDirection = calculateRelativeDirection(robotDirection, FRONT);
                    }

                    // Create the key for the map
                    TrafficLightSearchKey key = {trafficLightPosition, relativeDirection};

                    // Check if the key exists before adding
                    if (trafficLightMap.find(key) == trafficLightMap.end()) {
                        trafficLightMap[key] = {trafficLightRingPosition, processedTrafficLight.color};
                    } else {
                        // Optionally handle duplicates here
                        std::cout << "Duplicate entry for TrafficLightPosition: "
                                  << static_cast<int>(trafficLightPosition) << ", Direction: "
                                  << static_cast<int>(relativeDirection) << "\n";
                    }
                }
            }
        } else if (turnDirection == TurnDirection::COUNTER_CLOCKWISE) {
            if (!isnan(rightWallDistance)) {
                for (auto processedTrafficLight : processedTrafficLights) {
                    if (processedTrafficLight.size < 600) continue;

                    float trafficLightDistanceFromRight = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, rightWall));
                    float trafficLightDistanceFromFront = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, frontWall));

                    TrafficLightPosition trafficLightPosition;
                    TrafficLightRingPosition trafficLightRingPosition;
                    Direction relativeDirection;

                    if (trafficLightDistanceFromRight < 0.900) {
                        // Determine TrafficLightPosition based on distance from the front
                        if (trafficLightDistanceFromFront > 0.80 && trafficLightDistanceFromFront < 1.20) {
                            trafficLightPosition = TrafficLightPosition::BLUE;
                        } else if (trafficLightDistanceFromFront > 1.30 && trafficLightDistanceFromFront < 1.70) {
                            trafficLightPosition = TrafficLightPosition::MID;
                        } else if (trafficLightDistanceFromFront > 1.80 && trafficLightDistanceFromFront < 2.20) {
                            trafficLightPosition = TrafficLightPosition::ORANGE;
                        }

                        // Determine TrafficLightRingPosition
                        if (trafficLightDistanceFromRight < 0.500) {
                            trafficLightRingPosition = TrafficLightRingPosition::OUTER;
                        } else {
                            trafficLightRingPosition = TrafficLightRingPosition::INNER;
                        }

                        relativeDirection = calculateRelativeDirection(robotDirection, RIGHT);
                    } else {
                        // Determine TrafficLightPosition based on distance from the left
                        if (trafficLightDistanceFromRight >= 0.900 && trafficLightDistanceFromRight < 1.20) {
                            trafficLightPosition = TrafficLightPosition::ORANGE;
                        } else if (trafficLightDistanceFromRight > 1.30 && trafficLightDistanceFromRight < 1.70) {
                            trafficLightPosition = TrafficLightPosition::MID;
                        } else if (trafficLightDistanceFromRight > 1.80 && trafficLightDistanceFromRight < 2.20) {
                            trafficLightPosition = TrafficLightPosition::BLUE;
                        }

                        // Determine TrafficLightRingPosition
                        if (trafficLightDistanceFromFront < 0.500) {
                            trafficLightRingPosition = TrafficLightRingPosition::OUTER;
                        } else {
                            trafficLightRingPosition = TrafficLightRingPosition::INNER;
                        }

                        relativeDirection = calculateRelativeDirection(robotDirection, FRONT);
                    }

                    // Create the key for the map
                    TrafficLightSearchKey key = {trafficLightPosition, relativeDirection};

                    // Check if the key exists before adding
                    if (trafficLightMap.find(key) == trafficLightMap.end()) {
                        trafficLightMap[key] = {trafficLightRingPosition, processedTrafficLight.color};
                    } else {
                        // Optionally handle duplicates here
                        std::cout << "Duplicate entry for TrafficLightPosition: "
                                  << static_cast<int>(trafficLightPosition) << ", Direction: "
                                  << static_cast<int>(relativeDirection) << "\n";
                    }
                }
            }
        }
    }

    float frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TURN_THRESHOLD;  // TODO: FIX THIS SHIT
    switch (state) {
    NORMAL_STATE: {
        case State::NORMAL:
            motorPercent = 0.32;

            if (not isnan(frontWallDistance)) {
                if (frontWallDistance <= FRONT_WALL_DISTANCE_STOP_THRESHOLD && currentTime - lastTurnTime >= STOP_COOLDOWN && numberofTurn >= 3 * 4) {
                    state = State::STOP;
                    goto STOP_STATE;
                }
                if (frontWallDistance <= FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
                    state = State::SLOW_BEFORE_TURN;
                    goto SLOW_BEFORE_TURN_STATE;
                }

                if (currentTime - lastTrafficTime > TRAFFIC_COOLDOWN && turnDirection != TurnDirection::UNKNOWN) {
                    TrafficLightPosition trafficLightPositionStart;
                    TrafficLightPosition trafficLightPositionEnd;
                    Direction relativeDirection;

                    TrafficLightRingPosition trafficLightRingPositionOnLeft;

                    if (turnDirection == CLOCKWISE) {
                        relativeDirection = calculateRelativeDirection(robotDirection, LEFT);
                        trafficLightPositionStart = TrafficLightPosition::BLUE;
                        trafficLightPositionEnd = TrafficLightPosition::ORANGE;
                        trafficLightRingPositionOnLeft = TrafficLightRingPosition::OUTER;
                    } else if (turnDirection == COUNTER_CLOCKWISE) {
                        relativeDirection = calculateRelativeDirection(robotDirection, RIGHT);
                        trafficLightPositionStart = TrafficLightPosition::ORANGE;
                        trafficLightPositionEnd = TrafficLightPosition::BLUE;
                        trafficLightRingPositionOnLeft = TrafficLightRingPosition::INNER;
                    }

                    if (frontWallDistance > 1.00 && frontWallDistance <= 2.70) {
                        TrafficLightSearchKey key;
                        if (frontWallDistance > 2.00 && frontWallDistance <= 2.70) {
                            key = {trafficLightPositionStart, relativeDirection};
                        } else if (frontWallDistance > 1.50 && frontWallDistance <= 2.20) {
                            key = {TrafficLightPosition::MID, relativeDirection};
                        } else if (frontWallDistance > 1.00 && frontWallDistance <= 1.70) {
                            key = {trafficLightPositionEnd, relativeDirection};
                        }

                        if (trafficLightMap.find(key) != trafficLightMap.end()) {
                            lastTrafficTime = currentTime;
                            if (trafficLightMap[key].second == Color::RED) {
                                if (trafficLightMap[key].first == trafficLightRingPositionOnLeft) {
                                    wallDistanceBias = RED_LEFT_WALL_BIAS;
                                } else {
                                    wallDistanceBias = RED_RIGHT_WALL_BIAS;
                                }
                            } else if (trafficLightMap[key].second == Color::GREEN) {
                                if (trafficLightMap[key].first == trafficLightRingPositionOnLeft) {
                                    wallDistanceBias = GREEN_LEFT_WALL_BIAS;
                                } else {
                                    wallDistanceBias = GREEN_RIGHT_WALL_BIAS;
                                }
                            }
                        }
                    }
                }
            }

            float wallDistanceError = 0;
            if (turnDirection == CLOCKWISE) {
                if (not std::isnan(leftWallDistance)) {
                    wallDistanceError = wallDistanceBias - (leftWallDistance - 0.500);
                }
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                if (not std::isnan(rightWallDistance)) {
                    wallDistanceError = wallDistanceBias - (0.500 - rightWallDistance);
                }
            } else {
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
    SLOW_BEFORE_TURN_STATE: {
        case State::SLOW_BEFORE_TURN:
            motorPercent = 0.25f;
            if (not isnan(frontWallDistance)) {
                if (turnDirection == CLOCKWISE && (wallDistanceBias == RED_LEFT_WALL_BIAS || wallDistanceBias == RED_RIGHT_WALL_BIAS)) {
                    ProcessedTrafficLight closestTrafficLightAfterTurn;
                    float closestTrafficLightAfterTurnDistance = std::numeric_limits<float>::max();
                    for (auto trafficLight : processedTrafficLights) {
                        // Calculate the angle between the center and the traffic light point in radians
                        float angleRadians = std::atan2(-(trafficLight.point.y - lidarCenter.y), trafficLight.point.x - lidarCenter.x);
                        float angleDegrees = angleRadians * (180 / M_PI);  // Convert to degrees
                        angleDegrees = fmod(90.0f - angleDegrees + 720.0f, 360.0f);

                        if (not(angleDegrees >= 0 && angleDegrees < 90)) continue;  // TODO: Remove this magic number

                        float distance = cv::norm(trafficLight.point - lidarCenter);
                        if (distance < closestTrafficLightAfterTurnDistance) {
                            closestTrafficLightAfterTurnDistance = distance;
                            closestTrafficLightAfterTurn = trafficLight;
                        }
                    }

                    float trafficLightAfterTurnDistanceFromCenter = toMeter(lidarScale, pointLinePerpendicularDistance(closestTrafficLightAfterTurn.point, frontWall)) - 0.500;

                    if (closestTrafficLightAfterTurn.color == Color::RED) {
                        if (trafficLightAfterTurnDistanceFromCenter > 0.0) {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD;
                        } else {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD;
                        }
                        state = State::WAITING_FOR_TURN;
                    } else if (closestTrafficLightAfterTurn.color == Color::GREEN) {
                        if (trafficLightAfterTurnDistanceFromCenter > 0.0) {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD;
                        } else {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD;
                        }
                        state = State::WAITING_FOR_TURN;
                    }
                } else if (turnDirection == COUNTER_CLOCKWISE && (wallDistanceBias == GREEN_LEFT_WALL_BIAS || wallDistanceBias == GREEN_RIGHT_WALL_BIAS)) {
                    ProcessedTrafficLight closestTrafficLightAfterTurn;
                    float closestTrafficLightAfterTurnDistance = std::numeric_limits<float>::max();
                    for (auto trafficLight : processedTrafficLights) {
                        // Calculate the angle between the center and the traffic light point in radians
                        float angleRadians = std::atan2(-(trafficLight.point.y - lidarCenter.y), trafficLight.point.x - lidarCenter.x);
                        float angleDegrees = angleRadians * (180 / M_PI);  // Convert to degrees
                        angleDegrees = fmod(90.0f - angleDegrees + 720.0f, 360.0f);

                        if (not(angleDegrees >= 0 && angleDegrees < 90)) continue;  // TODO: Remove this magic number

                        float distance = cv::norm(trafficLight.point - lidarCenter);
                        if (distance < closestTrafficLightAfterTurnDistance) {
                            closestTrafficLightAfterTurnDistance = distance;
                            closestTrafficLightAfterTurn = trafficLight;
                        }
                    }

                    float trafficLightAfterTurnDistanceFromCenter = 0.500 - toMeter(lidarScale, pointLinePerpendicularDistance(closestTrafficLightAfterTurn.point, frontWall));

                    if (closestTrafficLightAfterTurn.color == Color::RED) {
                        if (trafficLightAfterTurnDistanceFromCenter > 0.0) {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD;
                        } else {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD;
                        }
                        state = State::WAITING_FOR_TURN;
                    } else if (closestTrafficLightAfterTurn.color == Color::GREEN) {
                        if (trafficLightAfterTurnDistanceFromCenter > 0.0) {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD;
                        } else {
                            frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD;
                        }
                        state = State::WAITING_FOR_TURN;
                    }
                }
            }
        case State::WAITING_FOR_TURN:
            if (frontWallDistance <= frontWallDistanceTurnThreshold) {
                state = State::TURNING;
                goto TURNING_STATE;
            }

            float wallDistanceError = 0;
            if (turnDirection == CLOCKWISE) {
                if (not std::isnan(leftWallDistance)) {
                    wallDistanceError = wallDistanceBias - (leftWallDistance - 0.500);
                }
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                if (not std::isnan(rightWallDistance)) {
                    wallDistanceError = wallDistanceBias - (0.500 - rightWallDistance);
                }
            } else {
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
    TURNING_STATE: {  // Run once just after changing state
        if (turnDirection == CLOCKWISE) {
            robotDirection = calculateRelativeDirection(robotDirection, RIGHT);
        } else if (turnDirection == COUNTER_CLOCKWISE) {
            robotDirection = calculateRelativeDirection(robotDirection, LEFT);
        }
        case State::TURNING:
            motorPercent = 0.30f;

            float desiredYaw = directionToHeading(robotDirection);
            float headingError = fmod(desiredYaw - gyroYaw + 360.0f + 180.0f, 360.0f) - 180.0f;
            steeringPercent = steeringPID.calculate(headingError, deltaTime);

            if (abs(headingError) < MAX_HEADING_ERROR_BEFORE_EXIT_TURNING) {
                wallDistanceBias = 0.000;  // TODO: Handle traffic light after turn
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
