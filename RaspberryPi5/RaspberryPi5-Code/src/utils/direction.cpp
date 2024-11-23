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
        case NORTH: return "NORTH";
        case EAST: return "EAST";
        case SOUTH: return "SOUTH";
        case WEST: return "WEST";
    }
    return "UNKNOWN";
}

float directionToHeading(Direction direction) {
    switch (direction) {
        case NORTH: return 0.0f;
        case EAST: return 90.0f;
        case SOUTH: return 180.0f;
        case WEST: return 270.0f;
    }
    return 0.0f;
}
