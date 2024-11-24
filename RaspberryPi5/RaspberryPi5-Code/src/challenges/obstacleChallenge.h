#ifndef OBSTACLECHALLENGE_H
#define OBSTACLECHALLENGE_H

#include <vector>
#include <opencv2/opencv.hpp>

#include "../utils/imageProcessor.h"
#include "../utils/lidarDataProcessor.h"
#include "../utils/PIDController.cpp"


class ObstacleChallenge {
private:
    PIDController steeringPID = PIDController(0.030f, 0.0f, 0.0005f);
    PIDController wallDistancePID = PIDController(130.0f, 0.0f, 0.0002f);

    const float MAX_HEADING_ERROR = 35.0;
    const float MIN_HEADING_ERROR = -35.0;

    const float FRONT_WALL_DISTANCE_STOP_THRESHOLD = 1.900;
    const float FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD = 1.100;
    const float FRONT_WALL_DISTANCE_TURN_THRESHOLD = 0.850;

    bool isRunning = true;

    float toLeftWallDistance = 0.500;

    float lastTurnTime = 0.0f; // Tracks when the last turn was made
    const float TURN_COOLDOWN = 2.5f; // Cooldown time in seconds
    const float STOP_COOLDOWN = 1.5f; // Cooldown time to stop after turn in seconds

    int scale;
    cv::Point center;          // Center point of the lidar map
    float initialGyroYaw;      // Initial yaw angle from the gyro for reference

    Direction direction = NORTH;
    TurnDirection turnDirection = UNKNOWN;
    int numberofTurn = 0;

public:
    /**
     * @brief Constructor to initialize the OpenChallenge class.
     * 
     * @param center Center point of the lidar map.
     * @param initialGyroYaw Initial yaw angle from the gyro.
     */
    ObstacleChallenge(int scale, cv::Point center, float initialGyroYaw);

    void update(const cv::Mat& lidarBinaryImage, const cv::Mat& cameraImage, float gyroYaw, float& motorPercent, float& steeringPercent);
};

#endif // OBSTACLECHALLENGE_H
