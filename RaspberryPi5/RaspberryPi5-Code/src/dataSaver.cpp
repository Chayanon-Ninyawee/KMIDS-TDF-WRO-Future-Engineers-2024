#include "dataSaver.h"

#include <fstream>
#include <iostream>

namespace DataSaver
{
  // Function to save scan data to a file
  bool saveData(const std::vector<lidarController::NodeData>& scanData, const std::string& filePath, bool append)
  {
    std::ofstream file(filePath, append ? std::ios::binary | std::ios::app : std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
      std::cerr << "Failed to open file for saving scan data: " << filePath << std::endl;
      return false;
    }

    // Save the size of the data vector first
    size_t dataSize = scanData.size();
    file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

    // Save the actual data
    file.write(reinterpret_cast<const char*>(scanData.data()), dataSize * sizeof(lidarController::NodeData));

    if (!file)
    {
      std::cerr << "Failed to save scan data to file." << std::endl;
      return false;
    }

    return true;
  }

  // Function to load all scan data from a file
  std::vector<std::vector<lidarController::NodeData>> loadData(const std::string& filePath)
  {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open())
    {
      std::cerr << "Failed to open file for loading scan data: " << filePath << std::endl;
      return {};
    }

    std::vector<std::vector<lidarController::NodeData>> allScanData;

    // Read data in chunks
    while (file)
    {
      size_t dataSize = 0;
      file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
      if (!file)
        break;

      std::vector<lidarController::NodeData> scanData(dataSize);
      file.read(reinterpret_cast<char*>(scanData.data()), dataSize * sizeof(lidarController::NodeData));
      if (!file)
        break;

      allScanData.push_back(std::move(scanData));
    }

    return allScanData;
  }
}
