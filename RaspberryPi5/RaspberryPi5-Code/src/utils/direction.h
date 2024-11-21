#ifndef DIRECTION_H
#define DIRECTION_H

#include <string>

// Enum representing the four cardinal directions
enum Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

// Enum representing the relative direction moves
enum RelativeDirection {
    FRONT,
    BACK,
    LEFT,
    RIGHT
};

// Function to calculate the new direction based on the current direction and relative move
Direction calculateRelativeDirection(Direction currentDirection, RelativeDirection relativeMove);

// Helper function to convert Direction enum to string for display
std::string directionToString(Direction direction);

float directionToHeading(Direction direction);

#endif // DIRECTION_H
