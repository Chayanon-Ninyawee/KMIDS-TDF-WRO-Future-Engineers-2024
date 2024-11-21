#ifndef DATASAVER_H
#define DATASAVER_H

#include <string>
#include <vector>

#include "lidarController.h"  // For NodeData
#include "i2c_master.h"

namespace DataSaver {

bool saveData(const std::string& filePath, const uint8_t calibData[22], bool append = true);
bool loadData(const std::string& filePath, uint8_t calibData[22]);

bool saveLogData(const std::string& filePath, const std::vector<lidarController::NodeData>& scanData, const bno055_accel_float_t& accel_data, const bno055_euler_float_t& euler_data, bool append = true);
bool loadLogData(const std::string& filePath, std::vector<std::vector<lidarController::NodeData>>& allScanData, std::vector<bno055_accel_float_t>& allAccelData, std::vector<bno055_euler_float_t>& allEulerData);

}  // namespace DataSaver

#endif  // DATASAVER_H
