#ifndef OBSTACLECHALLENGE_H
#define OBSTACLECHALLENGE_H

#include <vector>
#include <opencv2/opencv.hpp>

#include "../utils/imageProcessor.h"
#include "../utils/lidarDataProcessor.h"
#include "../utils/PIDController.cpp"

enum class State {
    STOP,
    NORMAL,
    SLOW_BEFORE_TURN,
    TURNING,
    UTURNING
};

class ObstacleChallenge {
private:
    PIDController steeringPID = PIDController(0.030f, 0.0f, 0.0005f);
    PIDController wallDistancePID = PIDController(130.0f, 0.0f, 0.0002f);

    const float MAX_HEADING_ERROR = 35.0;
    const float MIN_HEADING_ERROR = -35.0;

    const float FRONT_WALL_DISTANCE_STOP_THRESHOLD = 1.900;
    const float FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD = 1.200;
    const float FRONT_WALL_DISTANCE_TURN_THRESHOLD = 0.800;

    const float FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD = 0.950;
    const float FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD = 0.900;
    const float FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD = 0.600;
    const float FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD = 0.650;

    const float RED_RIGHT_WALL_BIAS = 0.350;
    const float RED_LEFT_WALL_BIAS = 0.150;
    const float GREEN_RIGHT_WALL_BIAS = -0.150;
    const float GREEN_LEFT_WALL_BIAS = -0.350;

    const float MAX_HEADING_ERROR_BEFORE_EXIT_TURNING = 5.0;

    float lastTurnTime = 0.0f; // Tracks when the last turn was made
    const float TURN_COOLDOWN = 2.5f; // Cooldown time in seconds
    const float STOP_COOLDOWN = 1.5f; // Cooldown time to stop after turn in seconds

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
     * @param lidarCenter Center point of the lidar map.
     * @param initialGyroYaw Initial yaw angle from the gyro.
     */
    ObstacleChallenge(int lidarScale, cv::Point lidarCenter);

    void update(const cv::Mat& lidarBinaryImage, const cv::Mat& cameraImage, float gyroYaw, float& motorPercent, float& steeringPercent);
};

#endif // OBSTACLECHALLENGE_H
