#ifndef OPENCHALLENGE_H
#define OPENCHALLENGE_H

#include <vector>
#include <opencv2/opencv.hpp>

#include "../utils/lidarDataProcessor.h"
#include "../utils/PIDController.cpp"

enum class State {
    STOP,
    NORMAL,
    TURNING
};

class OpenChallenge {
private:
    PIDController steeringPID = PIDController(0.017f, 0.0f, 0.0019f);
    PIDController wallDistancePID = PIDController(50.0f, 0.0f, 0.0030f);

    const float MAX_HEADING_ERROR = 25.0;
    const float MIN_HEADING_ERROR = -25.0;

    const float FRONT_WALL_DISTANCE_STOP_THRESHOLD = 2.200;
    const float FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD = 1.000;
    const float FRONT_WALL_DISTANCE_TURN_THRESHOLD = 0.690;

    const float OUTER_WALL_DISTANCE = 0.350;

    const float MAX_HEADING_ERROR_BEFORE_EXIT_TURNING = 10.0;

    float lastTurnTime = -1.0f; // Tracks when the last turn was made
    const float TURN_COOLDOWN = 1.0f; // Cooldown time in seconds
    const float STOP_COOLDOWN = 0.2f; // Cooldown time to stop after turn in seconds

    State state = State::NORMAL;

    float wallDistanceBias = 0.000; // Negative is left bias and positive is right bias

    int lidarScale;
    cv::Point lidarCenter;          // Center point of the lidar map

    Direction robotDirection = NORTH;
    TurnDirection turnDirection = UNKNOWN;
    int numberofTurn = 0;

public:
    /**
     * @brief Constructor to initialize the OpenChallenge class.
     * 
     * @param center Center point of the lidar map.
     * @param initialGyroYaw Initial yaw angle from the gyro.
     */
    OpenChallenge(int lidarScale, cv::Point lidarCenter);

    /**
     * @brief Update motor and steering percentages based on detected lines and current gyro yaw.
     * 
     * @param combined_lines Vector of detected and combined lines.
     * @param gyroYaw Current yaw angle from the gyro.
     * @param motorPercent Output parameter for motor speed as a percentage (-1.0 to 1.0).
     * @param steeringPercent Output parameter for steering angle as a percentage (-1.0 to 1.0).
     */
    void update(const cv::Mat& lidarBinaryImage, float gyroYaw, float& motorPercent, float& steeringPercent);
};

#endif // OPENCHALLENGE_H
