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
    PIDController steeringPID = PIDController(0.020f, 0.0f, 0.0f);

    const float innerWallDistance = 0.200;
    const float frontWallDistanceTurnThreshold = 0.900;

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
