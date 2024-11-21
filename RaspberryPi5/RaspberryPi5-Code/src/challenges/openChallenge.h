#ifndef OPENCHALLENGE_H
#define OPENCHALLENGE_H

#include <vector>
#include <opencv2/opencv.hpp>

#include "../utils/lidarDataProcessor.h"
#include "../utils/PIDController.cpp"


enum TurnDirection {
    CLOCKWISE,
    COUNTER_CLOCKWISE,
    UNKNOWN
};


class OpenChallenge {
private:
    PIDController steeringPID = PIDController(0.009f, 0.0f, 0.0f);
    PIDController wallDistancePID = PIDController(150.0f, 0.0f, 0.0f);

    const float MAX_HEADING_ERROR = 25.0;
    const float MIN_HEADING_ERROR = -25.0;

    const float INNER_WALL_DISTANCE = 0.350;
    const float FRONT_WALL_DISTANCE_TURN_THRESHOLD = 1.280;

    float lastTurnTime = 0.0f; // Tracks when the last turn was made
    const float TURN_COOLDOWN = 1.0f; // Cooldown time in seconds

    int scale;
    cv::Point center;          // Center point of the lidar map
    float initialGyroYaw;      // Initial yaw angle from the gyro for reference

    Direction direction = NORTH;
    TurnDirection turnDirection = COUNTER_CLOCKWISE;
    int numberofTurn = 0;

public:
    /**
     * @brief Constructor to initialize the OpenChallenge class.
     * 
     * @param center Center point of the lidar map.
     * @param initialGyroYaw Initial yaw angle from the gyro.
     */
    OpenChallenge(int scale, cv::Point center, float initialGyroYaw);

    /**
     * @brief Update motor and steering percentages based on detected lines and current gyro yaw.
     * 
     * @param combined_lines Vector of detected and combined lines.
     * @param gyroYaw Current yaw angle from the gyro.
     * @param motorPercent Output parameter for motor speed as a percentage (-1.0 to 1.0).
     * @param steeringPercent Output parameter for steering angle as a percentage (-1.0 to 1.0).
     */
    void update(const std::vector<cv::Vec4i>& combined_lines, float gyroYaw, float& motorPercent, float& steeringPercent);
};

#endif // OPENCHALLENGE_H
