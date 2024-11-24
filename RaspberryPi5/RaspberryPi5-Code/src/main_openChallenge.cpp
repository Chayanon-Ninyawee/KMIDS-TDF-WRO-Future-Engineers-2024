#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <thread>
#include <vector>

#include "challenges/openChallenge.h"

#include "utils/i2c_master.h"
#include "utils/lccv.hpp"
#include "utils/lidarController.h"
#include "utils/lidarDataProcessor.h"
#include "utils/dataSaver.h"

const uint8_t PICO_ADDRESS = 0x39;

float motorPercent = 0.0f;
float steeringPercent = 0.0f;


const int WIDTH = 1200;
const int HEIGHT = 1200;
const float LIDAR_SCALE = 180.0;

const cv::Point CENTER(WIDTH/2, HEIGHT/2);


uint32_t camWidth = 1296;
uint32_t camHeight = 972;


float lastGyroYaw = 0.0f;
float accumulateGyroYaw = 0.0f;

bool isRunning = true;

void interuptHandler(int signum) {
    isRunning = false;
}


// Draw all lines with different colors based on direction (NORTH, EAST, SOUTH, WEST)
void drawAllLines(const std::vector<cv::Vec4i> &lines, cv::Mat &outputImage, double gyroYaw) {
    // Analyze the direction of each wall using the analyzeWallDirection function
    std::vector<Direction> wallDirections = analyzeWallDirection(lines, gyroYaw, CENTER);

    for (size_t i = 0; i < lines.size(); ++i) {
        cv::Vec4i line = lines[i];
        Direction direction = wallDirections[i];  // Get the direction of the current line

        // Determine the color based on the direction
        cv::Scalar color;
        if (direction == NORTH) {
            color = cv::Scalar(0, 0, 255); // Red for NORTH
        } else if (direction == EAST) {
            color = cv::Scalar(0, 255, 0); // Green for EAST
        } else if (direction == SOUTH) {
            color = cv::Scalar(255, 0, 0); // Blue for SOUTH
        } else if (direction == WEST) {
            color = cv::Scalar(255, 255, 0); // Yellow for WEST
        }

        // Draw the line with the determined color
        cv::line(outputImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), color, 2, cv::LINE_AA);
    }
}


int main() {
    signal(SIGINT, interuptHandler);

    // cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);


    lccv::PiCamera cam;
    cam.options->video_width = camWidth;
    cam.options->video_height = camHeight;
    cam.options->framerate = 10;
    cam.options->brightness = 0.2;
    cam.options->contrast = 1.6;
    cam.options->setExposureMode(Exposure_Modes::EXPOSURE_SHORT);
    cam.options->verbose = true;
    cam.startVideo();



    int fd = i2c_master_init(PICO_ADDRESS);

    uint8_t logs[i2c_slave_mem_addr::LOGS_BUFFER_SIZE] = {0};

    i2c_master_send_command(fd, Command::RESTART);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));



    uint8_t calib[22];
    bool isCalibDataExist = DataSaver::loadData("config/calibData.bin", calib);

    // if (isCalibDataExist) {
    //     i2c_master_send_data(fd, i2c_slave_mem_addr::BNO055_CALIB_ADDR, calib, sizeof(calib));
    //     i2c_master_send_command(fd, Command::CALIB_WITH_OFFSET);
    // } else {
    //     i2c_master_send_command(fd, Command::CALIB_NO_OFFSET);
    // }
    i2c_master_send_command(fd, Command::SKIP_CALIB);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    uint8_t status[i2c_slave_mem_addr::STATUS_SIZE] = {0};
    while (not(status[0] & (1 << 1))) {
        i2c_master_read_status(fd, status);

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // uint8_t new_calib[22];
    // i2c_master_read_bno055_calibration(fd, new_calib);

    // DataSaver::saveData("config/calibData.bin", new_calib, false);

    // for (int i = 0; i < sizeof(new_calib); i++) {
    //     printf("%x, ", new_calib[i]);
    // }
    // printf("\n");



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

    lastGyroYaw = initial_euler_data.h;

    OpenChallenge challenge = OpenChallenge (LIDAR_SCALE, CENTER, 0.0f);

    while (isRunning) {
        cv::Mat rawCameraImage;
        if(!cam.getVideoFrame(rawCameraImage, 1000)){
            std::cout<<"Timeout error"<<std::endl;
        }
        cv::Mat cameraImage;
        cv::flip(rawCameraImage, cameraImage, -1);
 


        bno055_accel_float_t accel_data;
        bno055_euler_float_t euler_data;
        i2c_master_read_bno055_accel_and_euler(fd, &accel_data, &euler_data);

        float deltaYaw = euler_data.h - lastGyroYaw;
        if (deltaYaw > 180.0f) {
            deltaYaw -= 360.0f;
        } else if (deltaYaw < -180.0f) {
            deltaYaw += 360.0f;
        }
        accumulateGyroYaw += deltaYaw;
        lastGyroYaw = euler_data.h;

        printf("accumulateGyroYaw: %.2f, test: %.2f, ", accumulateGyroYaw, fmod(accumulateGyroYaw*1.007274762 + 360.0f*20, 360.0f));


        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        auto lidarScanData = lidar.getScanData();
        cv::Mat binaryImage = lidarDataToImage(lidarScanData, WIDTH, HEIGHT, LIDAR_SCALE);
        auto lines = detectLines(binaryImage);
        auto combined_lines = combineAlignedLines(lines);


        // cv::Mat outputImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
        // cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);
        // drawAllLines(combined_lines, outputImage, fmod(euler_data.h - initial_euler_data.h + 360.0f, 360.0f));
        

        challenge.update(combined_lines, fmod(accumulateGyroYaw*1.007274762 + 360.0f*20, 360.0f), motorPercent, steeringPercent);


        // int cropHeight = static_cast<int>(cameraImage.rows * 0.50);
        // cv::Rect cropRegion(0, cropHeight, cameraImage.cols, cameraImage.rows - cropHeight);
        // cv::Mat croppedImage = cameraImage(cropRegion);
        // if (DataSaver::saveLogData("log/logData1.bin", lidarScanData, accel_data, euler_data, croppedImage)) {
        //     // std::cout << "Log data saved to file successfully." << std::endl;
        // } else {
        //     std::cerr << "Failed to save log data to file." << std::endl;
        // }


        // motorPercent = 0.0f;
        // steeringPercent = 0.0f;
        // Send movement data via I2C
        uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

        memcpy(movement, &motorPercent, sizeof(motorPercent));
        memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
        i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));



        // cv::imshow("libcamera-demo", cameraImage);
        // cv::imshow("LIDAR Hough Lines", outputImage);

        // char key = cv::waitKey(1);
        // if (key == 'q') {
        //     break;
        // }
    }

    motorPercent = 0.0f;
    steeringPercent = 0.0f;

    // Send movement data via I2C
    uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

    memcpy(movement, &motorPercent, sizeof(motorPercent));
    memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
    i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));

    cam.stopVideo();

    lidar.shutdown();

    return 0;
}
