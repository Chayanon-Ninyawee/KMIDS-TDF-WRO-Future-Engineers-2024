#include "direction.h"

// Function to calculate the new direction based on the current direction and relative move
Direction calculateRelativeDirection(Direction currentDirection, RelativeDirection relativeMove) {
    switch (relativeMove) {
        case FRONT:
            return currentDirection; // No change for FRONT
        case BACK:
            return static_cast<Direction>((currentDirection + 2) % 4); // Opposite direction
        case LEFT:
            return static_cast<Direction>((currentDirection + 3) % 4); // Turn left (counter-clockwise)
        case RIGHT:
            return static_cast<Direction>((currentDirection + 1) % 4); // Turn right (clockwise)
    }
    return currentDirection; // Default (should not happen)
}

// Helper function to convert Direction enum to string for display
std::string directionToString(Direction direction) {
    switch (direction) {
        case Direction::NORTH: return "Direction::NORTH";
        case Direction::EAST: return "Direction::EAST";
        case Direction::SOUTH: return "Direction::SOUTH";
        case Direction::WEST: return "Direction::WEST";
    }
    return "UNKNOWN";
}

float directionToHeading(Direction direction) {
    switch (direction) {
        case Direction::NORTH: return 0.0f;
        case Direction::EAST: return 90.0f;
        case Direction::SOUTH: return 180.0f;
        case Direction::WEST: return 270.0f;
    }
    return 0.0f;
}
