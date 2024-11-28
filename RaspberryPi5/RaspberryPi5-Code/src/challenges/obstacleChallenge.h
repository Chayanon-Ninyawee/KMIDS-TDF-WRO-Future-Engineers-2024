#ifndef OBSTACLECHALLENGE_H
#define OBSTACLECHALLENGE_H

#include <queue>
#include <unordered_set>
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
    UTURNING_1,
    UTURNING_2,
    FIND_PARKING_ZONE,
    PARKING_1,
    PARKING_2
};

enum class TrafficLightRingPosition {
    OUTER,
    INNER
};

enum class TrafficLightPosition {
    BLUE,
    MID,
    ORANGE,
    NO_POSITION
};

struct TrafficLightSearchKey {
    TrafficLightPosition lightPosition;
    Direction direction;

    // Custom operator< for use in std::map
    bool operator<(const TrafficLightSearchKey& other) const {
        return std::tie(lightPosition, direction) < std::tie(other.lightPosition, other.direction);
    }
};

// Hash function for TrafficLightSearchKey (for use in std::unordered_set)
namespace std {
    template <>
    struct hash<TrafficLightSearchKey> {
        size_t operator()(const TrafficLightSearchKey& key) const {
            // Combine the hashes of lightPosition and direction
            size_t h1 = std::hash<int>{}(static_cast<int>(key.lightPosition));
            size_t h2 = std::hash<int>{}(static_cast<int>(key.direction));
            return h1 ^ (h2 << 1);  // Combine the two hashes (simple XOR-based combining)
        }
    };
}

class ObstacleChallenge {
private:
    PIDController steeringPID = PIDController(0.026f, 0.0f, 0.0008f);
    PIDController wallDistancePID = PIDController(200.0f, 0.0010f, 0.00000f);

    const float MAX_HEADING_ERROR = 40.0;
    const float MIN_HEADING_ERROR = -40.0;

    const float FRONT_WALL_DISTANCE_FIND_PARKING_THRESHOLD = 2.100;
    const float FRONT_WALL_DISTANCE_SLOWDOWN_THRESHOLD = 1.300;
    const float FRONT_WALL_DISTANCE_UTURN_THRESHOLD = 0.500;
    const float FRONT_WALL_DISTANCE_TURN_THRESHOLD = 0.750;

    const float FRONT_WALL_DISTANCE_TIGHT_INNER_MORE_TURN_THRESHOLD = 1.020;
    const float FRONT_WALL_DISTANCE_TIGHT_INNER_LESS_TURN_THRESHOLD = 0.870;
    const float FRONT_WALL_DISTANCE_TIGHT_OUTER_MORE_TURN_THRESHOLD = 0.460;
    const float FRONT_WALL_DISTANCE_TIGHT_OUTER_LESS_TURN_THRESHOLD = 0.670;
    float frontWallDistanceTurnThreshold = FRONT_WALL_DISTANCE_TURN_THRESHOLD;

    const float RED_RIGHT_WALL_BIAS = 0.280;
    const float RED_LEFT_WALL_BIAS = 0.120;
    const float GREEN_RIGHT_WALL_BIAS = -0.120;
    const float GREEN_LEFT_WALL_BIAS = -0.280;

    const float MAX_HEADING_ERROR_BEFORE_EXIT_TURNING = 5.0;

    float lastTurnTime = 0.0f; // Tracks when the last turn was made
    const float TURN_COOLDOWN = 1.1f; // Cooldown time in seconds
    const float FIND_PARKING_COOLDOWN = 1.1f; // Cooldown time to stop after turn in seconds

    float lastTrafficTime = 0.0f; // Tracks when the last turn was made
    const float TRAFFIC_COOLDOWN = 0.8f; // Cooldown time in seconds

    State state = State::NORMAL;
    bool isFindParking = false;

    std::map<TrafficLightSearchKey, std::pair<TrafficLightRingPosition, Color>> trafficLightMap;
    std::queue<TrafficLightSearchKey> trafficLightOrderQueue;  // Maintains the order of insertion
    std::unordered_set<TrafficLightSearchKey> trafficLightKeySet; // For fast duplicate checking
    TrafficLightPosition lastTrafficLightPosition;
    Direction lastTrafficLightDirection;

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
