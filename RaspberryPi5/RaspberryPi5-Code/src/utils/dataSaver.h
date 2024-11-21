#ifndef DATASAVER_H
#define DATASAVER_H

#include <string>
#include <vector>

#include "lidarController.h"  // For NodeData

namespace DataSaver {

bool saveData(const uint8_t calibData[22], const std::string& filePath, bool append = true);
bool loadData(const std::string& filePath, uint8_t calibData[22]);

bool saveLogData(const std::vector<lidarController::NodeData>& scanData, const std::string& filePath, bool append = true);
bool loadLogData(const std::string& filePath, std::vector<std::vector<lidarController::NodeData>>& allScanData);

}  // namespace DataSaver

#endif  // DATASAVER_H
