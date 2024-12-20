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
}

float lastUpdateTime = static_cast<float>(cv::getTickCount() / cv::getTickFrequency());
void ObstacleChallenge::update(const cv::Mat& lidarBinaryImage, const cv::Mat& cameraImage, float gyroYaw, float& motorPercent, float& steeringPercent) {
    float currentTime = static_cast<float>(cv::getTickCount()) / cv::getTickFrequency();
    float deltaTime = currentTime - lastUpdateTime;

    /*
    TODO
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
                    if (not (processedTrafficLight.color == Color::RED or processedTrafficLight.color == Color::GREEN)) continue;
                    // if (processedTrafficLight.size < 600) continue;

                    float trafficLightDistanceFromLeft = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, leftWall));
                    float trafficLightDistanceFromFront = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, frontWall));

                    TrafficLightPosition trafficLightPosition;
                    TrafficLightRingPosition trafficLightRingPosition;
                    Direction relativeDirection;

                    if (trafficLightDistanceFromLeft < 0.900) {
                        // Determine TrafficLightPosition based on distance from the front
                        if (trafficLightDistanceFromFront > 0.85 && trafficLightDistanceFromFront < 1.15) {
                            trafficLightPosition = TrafficLightPosition::ORANGE;
                        } else if (trafficLightDistanceFromFront > 1.35 && trafficLightDistanceFromFront < 1.65) {
                            trafficLightPosition = TrafficLightPosition::MID;
                        } else if (trafficLightDistanceFromFront > 1.85 && trafficLightDistanceFromFront < 2.15) {
                            trafficLightPosition = TrafficLightPosition::BLUE;
                        } else {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION;
                        }

                        // Determine TrafficLightRingPosition
                        if (trafficLightDistanceFromLeft < 0.480) {
                            trafficLightRingPosition = TrafficLightRingPosition::OUTER;
                        } else if (trafficLightDistanceFromLeft > 0.520)  {
                            trafficLightRingPosition = TrafficLightRingPosition::INNER;
                        } else {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION; // TODO: MAKE THIS BETTER
                        }

                        relativeDirection = calculateRelativeDirection(robotDirection, LEFT);
                    } else {
                        // Determine TrafficLightPosition based on distance from the left
                        if (trafficLightDistanceFromLeft >= 0.900 && trafficLightDistanceFromLeft < 1.15) {
                            trafficLightPosition = TrafficLightPosition::BLUE;
                        } else if (trafficLightDistanceFromLeft > 1.30 && trafficLightDistanceFromLeft < 1.70) {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION;
                        } else if (trafficLightDistanceFromLeft > 1.80 && trafficLightDistanceFromLeft < 2.20) {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION;
                        } else {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION;
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
                    if (trafficLightMap.find(key) == trafficLightMap.end() && trafficLightPosition != TrafficLightPosition::NO_POSITION) {
                        trafficLightMap[key] = {trafficLightRingPosition, processedTrafficLight.color};

                        std::cout << "Traffic Light Map Contents:\n";
                        for (const auto& [key, value] : trafficLightMap) {
                            // Extract key components
                            int lightPosition = static_cast<int>(key.lightPosition);
                            int direction = static_cast<int>(key.direction);

                            // Extract value components
                            int ringPosition = static_cast<int>(value.first);
                            int color = static_cast<int>(value.second);

                            // Print the contents
                            std::cout << "TrafficLightPosition: " << lightPosition
                                    << ", Direction: " << direction
                                    << " -> TrafficLightRingPosition: " << ringPosition
                                    << ", Color: " << color << "\n";
                        }
                    } else {
                        // Optionally handle duplicates here
                        // std::cout << "Duplicate entry for TrafficLightPosition: "
                        //           << static_cast<int>(trafficLightPosition) << ", Direction: "
                        //           << static_cast<int>(relativeDirection) << "\n";
                    }
                }
            }
        } else if (turnDirection == TurnDirection::COUNTER_CLOCKWISE) {
            if (!isnan(rightWallDistance)) {
                for (auto processedTrafficLight : processedTrafficLights) {
                    if (not (processedTrafficLight.color == Color::RED or processedTrafficLight.color == Color::GREEN)) continue;
                    // if (processedTrafficLight.size < 600) continue;

                    float trafficLightDistanceFromRight = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, rightWall));
                    float trafficLightDistanceFromFront = toMeter(lidarScale, pointLinePerpendicularDistance(processedTrafficLight.point, frontWall));

                    TrafficLightPosition trafficLightPosition;
                    TrafficLightRingPosition trafficLightRingPosition;
                    Direction relativeDirection;

                    if (trafficLightDistanceFromRight < 0.900) {
                        // Determine TrafficLightPosition based on distance from the front
                        if (trafficLightDistanceFromFront > 0.80 && trafficLightDistanceFromFront < 1.15) {
                            trafficLightPosition = TrafficLightPosition::BLUE;
                        } else if (trafficLightDistanceFromFront > 1.35 && trafficLightDistanceFromFront < 1.65) {
                            trafficLightPosition = TrafficLightPosition::MID;
                        } else if (trafficLightDistanceFromFront > 1.85 && trafficLightDistanceFromFront < 2.15) {
                            trafficLightPosition = TrafficLightPosition::ORANGE;
                        } else {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION;
                        }

                        // Determine TrafficLightRingPosition
                        if (trafficLightDistanceFromRight < 0.480) {
                            trafficLightRingPosition = TrafficLightRingPosition::OUTER;
                        } else if (trafficLightDistanceFromRight > 0.520) {
                            trafficLightRingPosition = TrafficLightRingPosition::INNER;
                        } else {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION; // TODO: MAKE THIS BETTER
                        }

                        relativeDirection = calculateRelativeDirection(robotDirection, RIGHT);
                    } else {
                        // Determine TrafficLightPosition based on distance from the left
                        if (trafficLightDistanceFromRight >= 0.900 && trafficLightDistanceFromRight < 1.15) {
                            trafficLightPosition = TrafficLightPosition::ORANGE;
                        } else if (trafficLightDistanceFromRight > 1.30 && trafficLightDistanceFromRight < 1.70) {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION;
                        } else if (trafficLightDistanceFromRight > 1.80 && trafficLightDistanceFromRight < 2.20) {
                            trafficLightPosition = TrafficLightPosition::NO_POSITION;
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
                    if (trafficLightMap.find(key) == trafficLightMap.end() && trafficLightPosition != TrafficLightPosition::NO_POSITION) {
                        trafficLightMap[key] = {trafficLightRingPosition, processedTrafficLight.color};
                        
                        std::cout << "Traffic Light Map Contents:\n";
                        for (const auto& [key, value] : trafficLightMap) {
                            // Extract key components
                            int lightPosition = static_cast<int>(key.lightPosition);
                            int direction = static_cast<int>(key.direction);

                            // Extract value components
                            int ringPosition = static_cast<int>(value.first);
                            int color = static_cast<int>(value.second);

                            // Print the contents
                            std::cout << "TrafficLightPosition: " << lightPosition
                                    << ", Direction: " << direction
                                    << " -> TrafficLightRingPosition: " << ringPosition
                                    << ", Color: " << color << "\n";
                        }
                    } else {
                        // Optionally handle duplicates here
                        // std::cout << "Duplicate entry for TrafficLightPosition: "
                        //           << static_cast<int>(trafficLightPosition) << ", Direction: "
                        //           << static_cast<int>(relativeDirection) << "\n";
                    }
                }
            }
        }
    }

    switch (state) {
    NORMAL_STATE: {
        case State::NORMAL:
            motorPercent = 0.25;

            if (not isnan(frontWallDistance)) {
                bool isUturn = false;

                if (!trafficLightOrderQueue.empty()) {
                    if (trafficLightMap[trafficLightOrderQueue.back()].second == Color::RED) {
                        isUturn = true;
                    }
                }

                if (numberofTurn == 2*4 && isUturn) {
                    if (frontWallDistance <= FRONT_WALL_DISTANCE_UTURN_THRESHOLD && currentTime - lastTurnTime >= FIND_PARKING_COOLDOWN) {
                        state = State::UTURNING_1;
                        goto UTURNING_1_STATE;
                    }
                } else if (numberofTurn == 3*4) {
                    if (frontWallDistance <= FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD && currentTime - lastTurnTime >= FIND_PARKING_COOLDOWN) {
                        isFindParking = true;
                        state = State::TURNING;
                        goto TURNING_STATE;
                    }
                } else {
                    if (frontWallDistance <= FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD && currentTime - lastTurnTime >= TURN_COOLDOWN) {
                        state = State::SLOW_BEFORE_TURN;
                        goto SLOW_BEFORE_TURN_STATE;
                    }
                }

                if (turnDirection != TurnDirection::UNKNOWN) {
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

                    if (frontWallDistance > 1.00 && frontWallDistance <= 2.90) {
                        TrafficLightSearchKey key = {TrafficLightPosition::NO_POSITION, Direction::NORTH};
                        if (trafficLightMap.find(key) == trafficLightMap.end() && frontWallDistance > 2.00 && frontWallDistance <= 2.90 && not (lastTrafficLightPosition == trafficLightPositionStart && lastTrafficLightDirection == relativeDirection)) {
                            key = {trafficLightPositionStart, relativeDirection};
                        }
                        if (trafficLightMap.find(key) == trafficLightMap.end() && frontWallDistance > 1.50 && frontWallDistance <= 2.40 && not (lastTrafficLightPosition == TrafficLightPosition::MID && lastTrafficLightDirection == relativeDirection)) {
                            key = {TrafficLightPosition::MID, relativeDirection};
                        }
                        if (trafficLightMap.find(key) == trafficLightMap.end() && frontWallDistance > 1.00 && frontWallDistance <= 1.90 && not (lastTrafficLightPosition == trafficLightPositionEnd && lastTrafficLightDirection == relativeDirection)) {
                            key = {trafficLightPositionEnd, relativeDirection};
                        }

                        if (trafficLightMap.find(key) != trafficLightMap.end()) {
                            lastTrafficLightPosition = key.lightPosition;
                            lastTrafficLightDirection = key.direction;

                            if (trafficLightMap[key].second == Color::RED) {
                                if (trafficLightKeySet.find(key) == trafficLightKeySet.end()) {  // Check if it's not a duplicate
                                    trafficLightOrderQueue.push(key);  // Add the key to the queue to preserve the order
                                    trafficLightKeySet.insert(key);    // Mark the key as seen
                                }
                                if (trafficLightMap[key].first == trafficLightRingPositionOnLeft) {
                                    wallDistanceBias = RED_LEFT_WALL_BIAS;
                                } else {
                                    wallDistanceBias = RED_RIGHT_WALL_BIAS;
                                }
                            } else if (trafficLightMap[key].second == Color::GREEN) {
                                if (trafficLightKeySet.find(key) == trafficLightKeySet.end()) {  // Check if it's not a duplicate
                                    trafficLightOrderQueue.push(key);  // Add the key to the queue to preserve the order
                                    trafficLightKeySet.insert(key);    // Mark the key as seen
                                }
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

            // wallDistanceBias = RED_LEFT_WALL_BIAS;
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
                TrafficLightPosition trafficLightPositionStart;
                TrafficLightPosition trafficLightPositionEnd;
                Direction relativeDirection;

                TrafficLightRingPosition trafficLightRingPositionOnLeft;

                if (turnDirection == CLOCKWISE) {
                    relativeDirection = calculateRelativeDirection(robotDirection, FRONT);
                    trafficLightPositionStart = TrafficLightPosition::BLUE;
                    trafficLightPositionEnd = TrafficLightPosition::ORANGE;
                    trafficLightRingPositionOnLeft = TrafficLightRingPosition::OUTER;
                } else if (turnDirection == COUNTER_CLOCKWISE) {
                    relativeDirection = calculateRelativeDirection(robotDirection, FRONT);
                    trafficLightPositionStart = TrafficLightPosition::ORANGE;
                    trafficLightPositionEnd = TrafficLightPosition::BLUE;
                    trafficLightRingPositionOnLeft = TrafficLightRingPosition::INNER;
                }

                if (turnDirection == CLOCKWISE) {
                    TrafficLightSearchKey key = {trafficLightPositionStart, relativeDirection};

                    if (trafficLightMap.find(key) != trafficLightMap.end()) {
                        if (trafficLightMap[key].second == Color::RED) {
                            if (trafficLightKeySet.find(key) == trafficLightKeySet.end()) {  // Check if it's not a duplicate
                                trafficLightOrderQueue.push(key);  // Add the key to the queue to preserve the order
                                trafficLightKeySet.insert(key);    // Mark the key as seen
                            }
                            if (trafficLightMap[key].first == TrafficLightRingPosition::INNER) {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            } else {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            }
                            state = State::WAITING_FOR_TURN;
                        } else if (trafficLightMap[key].second == Color::GREEN) {
                            if (trafficLightKeySet.find(key) == trafficLightKeySet.end()) {  // Check if it's not a duplicate
                                trafficLightOrderQueue.push(key);  // Add the key to the queue to preserve the order
                                trafficLightKeySet.insert(key);    // Mark the key as seen
                            }
                            if (trafficLightMap[key].first == TrafficLightRingPosition::INNER) {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            } else {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            }
                            state = State::WAITING_FOR_TURN;
                        }
                    }
                } else if (turnDirection == COUNTER_CLOCKWISE) {
                    TrafficLightSearchKey key = {trafficLightPositionStart, relativeDirection};

                    if (trafficLightMap.find(key) != trafficLightMap.end()) {
                        if (trafficLightMap[key].second == Color::RED) {
                            if (trafficLightKeySet.find(key) == trafficLightKeySet.end()) {  // Check if it's not a duplicate
                                trafficLightOrderQueue.push(key);  // Add the key to the queue to preserve the order
                                trafficLightKeySet.insert(key);    // Mark the key as seen
                            }
                            if (trafficLightMap[key].first == TrafficLightRingPosition::INNER) {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            } else {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            }
                            state = State::WAITING_FOR_TURN;
                        } else if (trafficLightMap[key].second == Color::GREEN) {
                            if (trafficLightKeySet.find(key) == trafficLightKeySet.end()) {  // Check if it's not a duplicate
                                trafficLightOrderQueue.push(key);  // Add the key to the queue to preserve the order
                                trafficLightKeySet.insert(key);    // Mark the key as seen
                            }
                            if (trafficLightMap[key].first == TrafficLightRingPosition::INNER) {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            } else {
                                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD;
                                lastTrafficLightPosition = trafficLightPositionStart;
                                lastTrafficLightDirection = relativeDirection;
                            }
                            state = State::WAITING_FOR_TURN;
                        }
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
            motorPercent = 0.25f;

            float desiredYaw = directionToHeading(robotDirection);
            float headingError = fmod(desiredYaw - gyroYaw + 360.0f + 180.0f, 360.0f) - 180.0f;
            steeringPercent = steeringPID.calculate(headingError, deltaTime);

            if (abs(headingError) < MAX_HEADING_ERROR_BEFORE_EXIT_TURNING) {
                if (frontWallDistanceTurnThreshold == FRONT_WALL_DISTANCE_TURN_THRESHOLD) {
                    wallDistanceBias = 0.000;
                } else if (frontWallDistanceTurnThreshold == FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD) {
                    if (turnDirection == CLOCKWISE) {
                        wallDistanceBias = RED_RIGHT_WALL_BIAS;
                    } else if (turnDirection == COUNTER_CLOCKWISE) {
                        wallDistanceBias = GREEN_LEFT_WALL_BIAS;
                    }
                } else if (frontWallDistanceTurnThreshold == FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD) {
                    if (turnDirection == CLOCKWISE) {
                        wallDistanceBias = RED_LEFT_WALL_BIAS;
                    } else if (turnDirection == COUNTER_CLOCKWISE) {
                        wallDistanceBias = GREEN_RIGHT_WALL_BIAS;
                    }
                } else if (frontWallDistanceTurnThreshold == FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD) {
                    if (turnDirection == CLOCKWISE) {
                        wallDistanceBias = GREEN_RIGHT_WALL_BIAS;
                    } else if (turnDirection == COUNTER_CLOCKWISE) {
                        wallDistanceBias = RED_LEFT_WALL_BIAS;
                    }
                } else if (frontWallDistanceTurnThreshold == FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD) {
                    if (turnDirection == CLOCKWISE) {
                        wallDistanceBias = GREEN_LEFT_WALL_BIAS;
                    } else if (turnDirection == COUNTER_CLOCKWISE) {
                        wallDistanceBias = RED_RIGHT_WALL_BIAS;
                    }
                }
                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TURN_THRESHOLD;
                lastTurnTime = currentTime;
                numberofTurn++;

                if (isFindParking) {
                    state = State::FIND_PARKING_ZONE;
                    goto FIND_PARKING_ZONE_STATE;
                } else {
                    state = State::NORMAL;
                    goto NORMAL_STATE;
                }
            }
            break;
    }
    UTURNING_1_STATE: {
        case State::UTURNING_1:
            float yawCorrection = 0.0f;
            if (turnDirection == CLOCKWISE) {
                if (wallDistanceBias == RED_LEFT_WALL_BIAS || wallDistanceBias == RED_RIGHT_WALL_BIAS) {
                    yawCorrection = -80.0f;
                } else {
                    yawCorrection = 80.0f;
                }
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                if (wallDistanceBias == GREEN_LEFT_WALL_BIAS || wallDistanceBias == GREEN_RIGHT_WALL_BIAS) {
                    yawCorrection = 80.0f;
                } else {
                    yawCorrection = -80.0f;
                }
            }
            float desiredYaw = directionToHeading(robotDirection);
            desiredYaw += yawCorrection;

            float headingError = fmod(desiredYaw - gyroYaw + 360.0f + 180.0f, 360.0f) - 180.0f;
            steeringPercent = steeringPID.calculate(headingError, deltaTime);

            if (abs(headingError) < 30) {
                state = State::UTURNING_2;
                goto UTURNING_2_STATE;
            }
            break;
    }
    UTURNING_2_STATE: {
        case State::UTURNING_2:
            float desiredYaw = directionToHeading(robotDirection);
            desiredYaw += 180;

            float headingError = fmod(desiredYaw - gyroYaw + 360.0f + 180.0f, 360.0f) - 180.0f;
            steeringPercent = steeringPID.calculate(headingError, deltaTime);

            if (abs(headingError) < MAX_HEADING_ERROR_BEFORE_EXIT_TURNING) {
                if (turnDirection == CLOCKWISE) {
                    turnDirection = COUNTER_CLOCKWISE;
                } else if (turnDirection == COUNTER_CLOCKWISE) {
                    turnDirection = CLOCKWISE;
                }

                lastTrafficLightPosition = TrafficLightPosition::NO_POSITION;

                robotDirection = calculateRelativeDirection(robotDirection, BACK);
                frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TURN_THRESHOLD;
                lastTurnTime = currentTime;
                numberofTurn++;

                state = State::NORMAL;
                goto NORMAL_STATE;
            }
            break;
    }
    FIND_PARKING_ZONE_STATE: {
        case State::FIND_PARKING_ZONE:
            motorPercent = 0.25f;

            auto potentialParkingWalls = detectParkingZone(lidarBinaryImage, combinedLines, wallDirections, turnDirection, robotDirection);

            std::vector<cv::Vec4i> parkingWalls;
            for (const auto& potentialParkingWall : potentialParkingWalls) {
                float wallAngle = pointLinePerpendicularDirection(lidarCenter, potentialParkingWall);
                if (!(wallAngle > 70 && wallAngle < 110)) continue;

                // Extract points (x1, y1) and (x2, y2)
                int x1 = potentialParkingWall[0];
                int x2 = potentialParkingWall[2];

                // Find leftestX and rightestX
                int leftestX = std::min(x1, x2);
                int rightestX = std::max(x1, x2);

                if (turnDirection == CLOCKWISE) {
                    if (leftestX < lidarCenter.x && rightestX + 20 > lidarCenter.x) {
                        parkingWalls.push_back(potentialParkingWall);
                    }
                } else if (turnDirection == COUNTER_CLOCKWISE) {
                    if (leftestX - 20 < lidarCenter.x && rightestX > lidarCenter.x) {
                        parkingWalls.push_back(potentialParkingWall);
                    }
                }
            }

            for (auto parkingWall : parkingWalls) {
                printf("Found Park Wall!\n");
                if (pointLinePerpendicularDistance(lidarCenter, parkingWall) < 110) {
                    state = State::PARKING_1;
                    goto PARKING_1_STATE;
                }
            }


            if (not isnan(frontWallDistance)) {
                if (frontWallDistance <= FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD && currentTime - lastTurnTime >= FIND_PARKING_COOLDOWN) {
                    state = State::TURNING;
                    goto TURNING_STATE;
                }
            }

            if (turnDirection == CLOCKWISE) {
                wallDistanceBias = GREEN_LEFT_WALL_BIAS;
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                wallDistanceBias = RED_RIGHT_WALL_BIAS;
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
    PARKING_1_STATE: {
        case State::PARKING_1:
            motorPercent = 0.18;

            auto potentialParkingWalls = detectParkingZone(lidarBinaryImage, combinedLines, wallDirections, turnDirection, robotDirection);

            std::vector<cv::Vec4i> parkingWalls;
            for (const auto& potentialParkingWall : potentialParkingWalls) {
                float wallAngle = pointLinePerpendicularDirection(lidarCenter, potentialParkingWall);
                if (!(wallAngle > 250 && wallAngle < 290)) continue;

                // Extract points (x1, y1) and (x2, y2)
                int x1 = potentialParkingWall[0];
                int x2 = potentialParkingWall[2];

                // Find leftestX and rightestX
                int leftestX = std::min(x1, x2);
                int rightestX = std::max(x1, x2);

                if (turnDirection == CLOCKWISE) {
                    if (rightestX < lidarCenter.x) {
                        parkingWalls.push_back(potentialParkingWall);
                    }
                } else if (turnDirection == COUNTER_CLOCKWISE) {
                    if (leftestX > lidarCenter.x) {
                        parkingWalls.push_back(potentialParkingWall);
                    }
                }
            }

            for (auto parkingWall : parkingWalls) {
                printf("Found Park Wall!\n");
                float parkingWallAngle = pointLinePerpendicularDirection(lidarCenter, parkingWall);
                if (pointLinePerpendicularDistance(lidarCenter, parkingWall) > 62 && (parkingWallAngle > 250 && parkingWallAngle < 290)) {
                    state = State::PARKING_2;
                    goto PARKING_2_STATE;
                }
            }


            if (turnDirection == CLOCKWISE) {
                wallDistanceBias = -0.150;
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                wallDistanceBias = 0.150;
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
    PARKING_2_STATE: {
        case State::PARKING_2:
            motorPercent = -0.25;
            float desiredYaw = directionToHeading(robotDirection);

            if (turnDirection == CLOCKWISE) {
                desiredYaw += 94;
            } else if (turnDirection == COUNTER_CLOCKWISE) {
                desiredYaw -= 94;
            }

            float headingError = fmod(desiredYaw - gyroYaw + 360.0f + 180.0f, 360.0f) - 180.0f;
            steeringPercent = steeringPID.calculate(-headingError, deltaTime);

            if (abs(headingError) < MAX_HEADING_ERROR_BEFORE_EXIT_TURNING) {
                state = State::STOP;
                goto STOP_STATE;
            }
            break;
    }
    STOP_STATE: {
        case State::STOP:
            motorPercent = -0.10f;
            steeringPercent = 0.0f;
            break;
    }
    }

    lastUpdateTime = currentTime;
}
