#include "dataSaver.h"

#include <fstream>
#include <iostream>

namespace DataSaver {

bool saveData(const uint8_t calibData[22], const std::string& filePath, bool append) {
    std::ofstream file(filePath, append ? std::ios::binary | std::ios::app : std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving calibration data: " << filePath << std::endl;
        return false;
    }

    // Save the size of the calibration data (fixed size: 22 bytes)
    size_t dataSize = 22;
    file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

    // Save the actual calibration data
    file.write(reinterpret_cast<const char*>(calibData), dataSize);

    if (!file) {
        std::cerr << "Failed to save calibration data to file." << std::endl;
        return false;
    }

    return true;
}

bool loadData(const std::string& filePath, uint8_t calibData[22]) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for loading calibration data: " << filePath << std::endl;
        return false;
    }

    // Read the size of the calibration data
    size_t dataSize = 0;
    file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));
    if (!file || dataSize != 22) {
        std::cerr << "Invalid or corrupt calibration data size in file." << std::endl;
        return false;
    }

    // Read the actual calibration data
    file.read(reinterpret_cast<char*>(calibData), dataSize);
    if (!file) {
        std::cerr << "Failed to read calibration data from file." << std::endl;
        return false;
    }

    return true;
}

bool saveLogData(const std::vector<lidarController::NodeData>& scanData, const std::string& filePath, bool append) {
    std::ofstream file(filePath, append ? std::ios::binary | std::ios::app : std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving scan data: " << filePath << std::endl;
        return false;
    }

    // Save the size of the data vector first
    size_t dataSize = scanData.size();
    file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

    // Save the actual data
    file.write(reinterpret_cast<const char*>(scanData.data()), dataSize * sizeof(lidarController::NodeData));

    if (!file) {
        std::cerr << "Failed to save scan data to file." << std::endl;
        return false;
    }

    return true;
}

bool loadLogData(const std::string& filePath, std::vector<std::vector<lidarController::NodeData>>& allScanData) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for loading scan data: " << filePath << std::endl;
        return false;
    }

    allScanData.clear();  // Ensure the output container is empty before loading new data

    // Read data chunks
    while (file) {
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

    return true;
}

}  // namespace DataSaver
