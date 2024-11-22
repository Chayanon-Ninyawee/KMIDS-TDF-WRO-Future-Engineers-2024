#include "dataSaver.h"

#include <fstream>
#include <iostream>
#include <filesystem>  // For creating directories

namespace DataSaver {

bool createDirectoryIfNeeded(const std::string& filePath) {
    // Extract the directory path from the filePath
    std::filesystem::path dirPath = std::filesystem::path(filePath).parent_path();
    
    // Check if directory exists, if not, create it
    if (!std::filesystem::exists(dirPath)) {
        std::cerr << "Directory doesn't exist. Creating: " << dirPath.string() << std::endl;
        return std::filesystem::create_directories(dirPath);
    }
    return true;
}

bool saveData(const std::string& filePath, const uint8_t calibData[22], bool append) {
    // Create directory if needed
    if (!createDirectoryIfNeeded(filePath)) {
        std::cerr << "Failed to create directory for saving calibration data." << std::endl;
        return false;
    }

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

bool saveLogData(const std::string& filePath, 
                 const std::vector<lidarController::NodeData>& scanData, 
                 const bno055_accel_float_t& accel_data, 
                 const bno055_euler_float_t& euler_data, 
                 const cv::Mat& image, 
                 bool append) {
    // Create directory if needed
    if (!createDirectoryIfNeeded(filePath)) {
        std::cerr << "Failed to create directory for saving log data." << std::endl;
        return false;
    }

    std::ofstream file(filePath, append ? std::ios::binary | std::ios::app : std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving log data: " << filePath << std::endl;
        return false;
    }

    // Save scan data size and contents
    size_t dataSize = scanData.size();
    file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));
    file.write(reinterpret_cast<const char*>(scanData.data()), dataSize * sizeof(lidarController::NodeData));

    // Save accel and Euler data
    file.write(reinterpret_cast<const char*>(&accel_data), sizeof(bno055_accel_float_t));
    file.write(reinterpret_cast<const char*>(&euler_data), sizeof(bno055_euler_float_t));

    // Encode and save image data
    std::vector<uchar> buffer;
    cv::imencode(".png", image, buffer);  // Save as PNG
    size_t imgSize = buffer.size();
    file.write(reinterpret_cast<const char*>(&imgSize), sizeof(imgSize));  // Save image size
    file.write(reinterpret_cast<const char*>(buffer.data()), imgSize);     // Save image bytes

    if (!file) {
        std::cerr << "Failed to save log data to file." << std::endl;
        return false;
    }

    return true;
}


bool loadLogData(const std::string& filePath, 
                 std::vector<std::vector<lidarController::NodeData>>& allScanData, 
                 std::vector<bno055_accel_float_t>& allAccelData, 
                 std::vector<bno055_euler_float_t>& allEulerData, 
                 std::vector<cv::Mat>& allCameraImage) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for loading log data: " << filePath << std::endl;
        return false;
    }

    allScanData.clear();
    allAccelData.clear();
    allEulerData.clear();
    allCameraImage.clear();

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

        bno055_accel_float_t accel_data;
        file.read(reinterpret_cast<char*>(&accel_data), sizeof(bno055_accel_float_t));

        bno055_euler_float_t euler_data;
        file.read(reinterpret_cast<char*>(&euler_data), sizeof(bno055_euler_float_t));

        // Read image data
        size_t imgSize = 0;
        file.read(reinterpret_cast<char*>(&imgSize), sizeof(imgSize));
        if (!file)
            break;

        std::vector<uchar> buffer(imgSize);
        file.read(reinterpret_cast<char*>(buffer.data()), imgSize);
        if (!file)
            break;

        cv::Mat image = cv::imdecode(buffer, cv::IMREAD_UNCHANGED);
        if (image.empty()) {
            std::cerr << "Failed to decode image from file." << std::endl;
            return false;
        }

        // Store the loaded data
        allScanData.push_back(std::move(scanData));
        allAccelData.push_back(accel_data);
        allEulerData.push_back(euler_data);
        allCameraImage.push_back(image);
    }

    return true;
}


}  // namespace DataSaver
