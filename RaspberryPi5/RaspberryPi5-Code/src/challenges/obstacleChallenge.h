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
    WAITING_FOR_TURN,
    TURNING,
    UTURNING
};

class ObstacleChallenge {
private:
    PIDController steeringPID = PIDController(0.026f, 0.0f, 0.0010f);
    PIDController wallDistancePID = PIDController(200.0f, 0.0f, 0.00010f);

    const float MAX_HEADING_ERROR = 40.0;
    const float MIN_HEADING_ERROR = -40.0;

    const float FRONT_WALL_DISTANCE_STOP_THRESHOLD = 1.900;
    const float FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD = 1.300;
    const float FRONT_WALL_DISTANCE_TURN_THRESHOLD = 0.820;

    const float FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD = 1.040;
    const float FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD = 0.870;
    const float FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD = 0.500;
    const float FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD = 0.670;

    const float RED_RIGHT_WALL_BIAS = 0.250;
    const float RED_LEFT_WALL_BIAS = 0.150;
    const float GREEN_RIGHT_WALL_BIAS = -0.150;
    const float GREEN_LEFT_WALL_BIAS = -0.250;

    const float MAX_HEADING_ERROR_BEFORE_EXIT_TURNING = 5.0;

    float lastTurnTime = 0.0f; // Tracks when the last turn was made
    const float TURN_COOLDOWN = 0.6f; // Cooldown time in seconds
    const float STOP_COOLDOWN = 0.3f; // Cooldown time to stop after turn in seconds

    float lastTrafficTime = 0.0f; // Tracks when the last turn was made
    const float TRAFFIC_COOLDOWN = 0.8f; // Cooldown time in seconds

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
