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


LibCamera cam;
uint32_t camWidth = 1024;
uint32_t camHeight = 576;
uint32_t camStride;


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


    int ret = cam.initCamera();
    cam.configureStill(camWidth, camHeight, formats::RGB888, 1, Orientation::Rotate180);
    ControlList controls_;
    int64_t frame_time = 1000000 / 10;
	controls_.set(controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>({ frame_time, frame_time })); // Set frame rate
    controls_.set(controls::Brightness, 0.1); // Adjust the brightness of the output images, in the range -1.0 to 1.0
    controls_.set(controls::Contrast, 1.0); // Adjust the contrast of the output image, where 1.0 = normal contrast
    controls_.set(controls::ExposureTime, 20000);
    cam.set(controls_);

    if (ret) {
        cam.closeCamera();
        printf("Camera no working!");
        return -1;
    }

    bool flag;
    LibcameraOutData frameData;
    cam.startCamera();
    cam.VideoStream(&camWidth, &camHeight, &camStride);



    int fd = i2c_master_init(PICO_ADDRESS);

    uint8_t logs[i2c_slave_mem_addr::LOGS_BUFFER_SIZE] = {0};

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

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    uint8_t status[i2c_slave_mem_addr::STATUS_SIZE] = {0};
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
        flag = cam.readFrame(&frameData);
        if (!flag) {
            continue;
        }

        cv::Mat im(camHeight, camWidth, CV_8UC3, frameData.imageData, camStride);
 


        bno055_accel_float_t accel_data;
        bno055_euler_float_t euler_data;
        i2c_master_read_bno055_accel_and_euler(fd, &accel_data, &euler_data);

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        auto lidarScanData = lidar.getScanData();
        cv::Mat binaryImage = lidarDataToImage(lidarScanData, WIDTH, HEIGHT, SCALE);
        auto lines = detectLines(binaryImage);
        auto combined_lines = combineAlignedLines(lines);


        // cv::Mat outputImage = cv::Mat::zeros(HEIGHT, WIDTH, CV_8UC3);
        // cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);
        // drawAllLines(combined_lines, outputImage, fmod(euler_data.h - initial_euler_data.h + 360.0f, 360.0f));
        

        challenge.update(combined_lines, euler_data.h, motorPercent, steeringPercent);



        // Send movement data via I2C
        uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

        memcpy(movement, &motorPercent, sizeof(motorPercent));
        memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
        i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));


        if (DataSaver::saveLogData("log/logData2.bin", lidarScanData, accel_data, euler_data, im)) {
            std::cout << "Log data saved to file successfully." << std::endl;
        } else {
            std::cerr << "Failed to save log data to file." << std::endl;
        }


        // cv::imshow("libcamera-demo", im);
        // cv::imshow("LIDAR Hough Lines", outputImage);

        // char key = cv::waitKey(1);
        // if (key == 'q') {
        //     break;
        // }

        cam.returnFrameBuffer(frameData);
    }

    motorPercent = 0.0f;
    steeringPercent = 0.0f;

    // Send movement data via I2C
    uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

    memcpy(movement, &motorPercent, sizeof(motorPercent));
    memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
    i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));

    cam.stopCamera();
    cam.closeCamera();

    lidar.shutdown();

    return 0;
}
