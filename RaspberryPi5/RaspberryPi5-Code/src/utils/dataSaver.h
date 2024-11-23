#ifndef DATASAVER_H
#define DATASAVER_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#include "lidar_struct.h"
#include "bno055_struct.h"

namespace DataSaver {

bool saveData(const std::string& filePath, const uint8_t calibData[22], bool append = true);
bool loadData(const std::string& filePath, uint8_t calibData[22]);

bool saveLogData(const std::string& filePath, 
                 const std::vector<lidarController::NodeData>& scanData, 
                 const bno055_accel_float_t& accel_data, 
                 const bno055_euler_float_t& euler_data, 
                 const cv::Mat& image, 
                 bool append = true);
bool loadLogData(const std::string& filePath, 
                 std::vector<std::vector<lidarController::NodeData>>& allScanData, 
                 std::vector<bno055_accel_float_t>& allAccelData, 
                 std::vector<bno055_euler_float_t>& allEulerData, 
                 std::vector<cv::Mat>& allImages);

}  // namespace DataSaver

#endif  // DATASAVER_H
