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

bool isRunning = true;

void interuptHandler(int signum) {
    isRunning = false;
}



// Draw all lines with different colors
void drawAllLines(const std::vector<cv::Vec4i> &lines, cv::Mat &outputImage) {
    for (size_t i = 0; i < lines.size(); ++i) {
        // Generate a bright, contrasting color by using modulo values to avoid dark colors
        int r = ((i * 73) % 156) + 100;   // Red component, adjusted to stay above 100
        int g = ((i * 89) % 156) + 100;   // Green component, adjusted to stay above 100
        int b = ((i * 113) % 156) + 100;  // Blue component, adjusted to stay above 100

        cv::Scalar color(0, 255, 0);  // Ensure colors are bright and non-black
        cv::Vec4i line = lines[i];
        cv::line(outputImage, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), color, 2, cv::LINE_AA);
    }
}

int main(int argc, char **argv) {
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
        i2c_master_read_data(fd, i2c_slave_mem_addr::STATUS_ADDR, status, sizeof(status));

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    uint8_t new_calib[22];
    i2c_master_read_data(fd, i2c_slave_mem_addr::BNO055_CALIB_ADDR, calib, sizeof(calib));

    DataSaver::saveData(new_calib, "config/calibData.bin", false);

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

    const int width = 1200;
    const int height = 1200;
    const float scale = 180.0;

    cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);

    while (isRunning) {
        int64 start = cv::getTickCount();

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));

        auto lidarScanData = lidar.getScanData();
        // lidar.printScanData(lidarScanData);

        cv::Mat binaryImage = lidarDataToImage(lidarScanData, width, height, scale);
        cv::Mat outputImage = cv::Mat::zeros(height, width, CV_8UC3);
        cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);

        auto lines = detectLines(binaryImage);
        auto combined_lines = combineAlignedLines(lines);
        drawAllLines(combined_lines, outputImage);

        for (size_t i = 0; i < combined_lines.size(); ++i) {
            cv::Vec4i line = combined_lines[i];
            printf("Line: %d, (%d, %d), (%d, %d), angle: %.2f\n", i, line[0], line[1], line[2], line[3], calculateAngle(line));
        }

        cv::imshow("LIDAR Hough Lines", outputImage);

        char key = cv::waitKey(1);
        if (key == 'q') {
            break;
        }

        int64 end = cv::getTickCount();
        double duration = (end - start) / cv::getTickFrequency();
        double fps = 1.0 / duration;
        printf("FPS: %.3f, # of lines: %d\n", fps, combined_lines.size());
    }

    lidar.shutdown();
    cv::destroyAllWindows();

    return 0;
}
