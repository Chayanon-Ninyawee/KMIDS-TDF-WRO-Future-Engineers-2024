#ifndef DATASAVER_H
#define DATASAVER_H

#include <vector>
#include <string>
#include "lidarController.h" // For NodeData

namespace DataSaver
{
  // Function to save scan data to a file
  // Parameters:
  //   scanData - Vector containing LIDAR scan data
  //   filePath - File path where the data should be saved
  //   append - Flag to indicate if data should be appended (default is false)
  // Returns:
  //   true if data was successfully saved, false otherwise
  bool saveData(const std::vector<lidarController::NodeData>& scanData, const std::string& filePath, bool append = true);

  // Function to load all scan data from a file
  // Parameters:
  //   filePath - File path from which the data should be loaded
  // Returns:
  //   A vector of scan data, each represented as a vector of NodeData
  std::vector<std::vector<lidarController::NodeData>> loadData(const std::string& filePath);
}

#endif // DATASAVER_H
