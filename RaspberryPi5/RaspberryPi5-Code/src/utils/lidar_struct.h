#ifndef LIDAR_STRUCT_H
#define LIDAR_STRUCT_H

namespace lidarController {

/**
   * Structure to hold data for each LIDAR scan node.
   * Contains the angle and distance of the detected point.
   */
  struct NodeData {
    float angle;
    float distance;
  };

}

#endif  // LIDAR_STRUCT_H