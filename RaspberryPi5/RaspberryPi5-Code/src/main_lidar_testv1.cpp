#include <chrono>
#include <cmath>
#include <csignal>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <thread>
#include <vector>

#include "utils/i2c_master.h"
#include "utils/lidarController.h"
#include "utils/lidarDataProcessor.h"
#include "utils/dataSaver.h"

const uint8_t PICO_ADDRESS = 0x39;

const int WIDTH = 1200;
const int HEIGHT = 1200;
const float SCALE = 180.0;

const cv::Point CENTER(WIDTH/2, HEIGHT/2);

float motorPercent = 0.0f;
float steeringPercent = 0.0f;


bool isRunning = true;

void interuptHandler(int signum) {
    isRunning = false;
}



// Draw all lines with different colors based on direction (NORTH, EAST, SOUTH, WEST)
void drawAllLines(cv::Mat &outputImage, const std::vector<cv::Vec4i> &lines, const std::vector<Direction> &wallDirections) {
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

int main(int argc, char **argv) {
    signal(SIGINT, interuptHandler);

    int fd = i2c_master_init(PICO_ADDRESS);

    i2c_master_send_command(fd, Command::RESTART);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));


    i2c_master_send_command(fd, Command::SKIP_CALIB);

    // uint8_t calib[22];
    // bool isCalibDataExist = DataSaver::loadData("config/calibData.bin", calib);

    // if (isCalibDataExist) {
    //     i2c_master_send_data(fd, i2c_slave_mem_addr::BNO055_CALIB_ADDR, calib, sizeof(calib));
    //     i2c_master_send_command(fd, Command::CALIB_WITH_OFFSET);
    // } else {
    //     i2c_master_send_command(fd, Command::CALIB_NO_OFFSET);
    // }

    uint8_t status[i2c_slave_mem_addr::STATUS_SIZE] = {0};
    uint8_t logs[i2c_slave_mem_addr::LOGS_BUFFER_SIZE] = {0};
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

    cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);

    while (isRunning) {
        int64 start = cv::getTickCount();

        // Send movement data via I2C
        uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

        memcpy(movement, &motorPercent, sizeof(motorPercent));
        memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
        i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));

        bno055_accel_float_t accel_data;
        bno055_euler_float_t euler_data;
        i2c_master_read_bno055_accel_and_euler(fd, &accel_data, &euler_data);

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        auto lidarScanData = lidar.getScanData();
        // lidar.printScanData(lidarScanData);

        cv::Mat binaryImage = lidarDataToImage(lidarScanData, WIDTH, HEIGHT, SCALE);
        cv::Mat outputImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
        cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);

        auto angle = 0;

        auto lines = detectLines(binaryImage);
        auto combinedLines = combineAlignedLines(lines);
        auto wallDirections = analyzeWallDirection(combinedLines, angle, CENTER);

        drawAllLines(outputImage, combinedLines, wallDirections);

        
        Direction direction = NORTH;

        if (angle >= 337.5 || angle < 22.5) direction = NORTH;
        else if (angle >= 22.5 && angle < 112.5) direction = EAST;
        else if (angle >= 112.5 && angle < 202.5) direction = SOUTH;
        else if (angle >= 202.5 && angle < 292.5) direction = WEST;
        else direction = NORTH; // Handles wrap-around for safety

        auto trafficLightPoints = detectTrafficLight(binaryImage, combinedLines, wallDirections, COUNTER_CLOCKWISE, direction);

        for (const auto point : trafficLightPoints) {
            cv::circle(outputImage, point, 5, cv::Scalar(255, 120, 255), cv::FILLED);
        }

        cv::circle(outputImage, CENTER, 10, cv::Scalar(0, 120, 255), cv::FILLED);

        cv::imshow("LIDAR Hough Lines", outputImage);

        char key = cv::waitKey(1);
        if (key == 'q') {
            break;
        }

        int64 end = cv::getTickCount();
        double duration = (end - start) / cv::getTickFrequency();
        double fps = 1.0 / duration;
        printf("FPS: %.3f, # of lines: %d\n", fps, combinedLines.size());
    }

    lidar.shutdown();
    cv::destroyAllWindows();

    return 0;
}
