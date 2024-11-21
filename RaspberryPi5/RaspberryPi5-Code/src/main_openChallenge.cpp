#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "challenges/openChallenge.h"

#include "utils/i2c_master.h"
#include "utils/libCamera.h"
#include "utils/lidarController.h"
#include "utils/lidarDataProcessor.h"
#include "utils/dataSaver.h"

const uint8_t PICO_ADDRESS = 0x39;

float motorPercent = 0.0f;
float steeringPercent = 0.0f;


const int WIDTH = 1200;
const int HEIGHT = 1200;
const float SCALE = 180.0;

const cv::Point CENTER(WIDTH/2, HEIGHT/2);


bool isRunning = true;

void interuptHandler(int signum) {
    isRunning = false;
}



int main() {
    signal(SIGINT, interuptHandler);

    int fd = i2c_master_init(PICO_ADDRESS);

    i2c_master_send_command(fd, Command::RESTART);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    uint8_t calib[22];
    bool isCalibDataExist = DataSaver::loadData("config/calibData.bin", calib);

    if (isCalibDataExist) {
        i2c_master_send_data(fd, i2c_slave_mem_addr::BNO055_CALIB_ADDR, calib, sizeof(calib));
        i2c_master_send_command(fd, Command::CALIB_WITH_OFFSET);
    } else {
        i2c_master_send_command(fd, Command::CALIB_NO_OFFSET);
    }

    uint8_t status[i2c_slave_mem_addr::STATUS_SIZE] = {0};
    uint8_t logs[i2c_slave_mem_addr::LOGS_BUFFER_SIZE] = {0};
    while (not(status[0] & (1 << 1))) {
        i2c_master_read_status(fd, status);

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    uint8_t new_calib[22];
    i2c_master_read_bno055_calibration(fd, new_calib);

    DataSaver::saveData("config/calibData.bin", new_calib, false);

    for (int i = 0; i < sizeof(new_calib); i++) {
        printf("%x, ", new_calib[i]);
    }
    printf("\n");

    i2c_master_send_command(fd, Command::NO_COMMAND);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));


    lidarController::LidarController lidar;
    if (!lidar.initialize() || !lidar.startScanning()) {
        return -1;
    }

    printf("Press Any Key to Start\n");  
    getchar();

    bno055_accel_float_t initial_accel_data;
    bno055_euler_float_t initial_euler_data;
    i2c_master_read_bno055_accel_and_euler(fd, &initial_accel_data, &initial_euler_data);

    OpenChallenge challenge = OpenChallenge (SCALE, CENTER, initial_euler_data.h);

    while (isRunning) {
        bno055_accel_float_t accel_data;
        bno055_euler_float_t euler_data;
        i2c_master_read_bno055_accel_and_euler(fd, &accel_data, &euler_data);

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        auto lidarScanData = lidar.getScanData();
        cv::Mat binaryImage = lidarDataToImage(lidarScanData, WIDTH, HEIGHT, SCALE);
        auto lines = detectLines(binaryImage);
        auto combined_lines = combineAlignedLines(lines);

        

        challenge.update(combined_lines, euler_data.h, motorPercent, steeringPercent);



        // Send movement data via I2C
        uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

        memcpy(movement, &motorPercent, sizeof(motorPercent));
        memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
        i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));


        if (DataSaver::saveLogData("log/logData2.bin", lidarScanData, accel_data, euler_data)) {
            std::cout << "Log data saved to file successfully." << std::endl;
        } else {
            std::cerr << "Failed to save log data to file." << std::endl;
        }
    }

    motorPercent = 0.0f;
    steeringPercent = 0.0f;

    // Send movement data via I2C
    uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

    memcpy(movement, &motorPercent, sizeof(motorPercent));
    memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
    i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));

    lidar.shutdown();

    return 0;
}
