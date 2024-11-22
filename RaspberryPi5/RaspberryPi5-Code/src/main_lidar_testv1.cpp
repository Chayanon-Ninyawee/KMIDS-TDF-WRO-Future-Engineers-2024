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
#include "utils/libCamera.h"
#include "utils/imageProcessor.h"
#include "utils/dataSaver.h"

const uint8_t PICO_ADDRESS = 0x39;

const int LIDAR_WIDTH = 1200;
const int LIDAR_HEIGHT = 1200;
const float LIDAR_SCALE = 180.0;

const cv::Point CENTER(LIDAR_WIDTH/2, LIDAR_HEIGHT/2);

LibCamera cam;
uint32_t camWidth = 1280;
uint32_t camHeight = 720;
uint32_t camStride;


float motorPercent = 0.0f;
float steeringPercent = 0.0f;


bool isRunning = true;







// Global variable to store the clicked point coordinates
cv::Point clickPoint(-1, -1);

// Mouse callback function to capture the click event
void onMouse(int event, int x, int y, int flags, void* userdata) {
    if (event == cv::EVENT_LBUTTONDOWN) {
        // Store the clicked point
        clickPoint = cv::Point(x, y);
    }
}










void interuptHandler(int signum) {
    isRunning = false;
}


int main(int argc, char **argv) {
    signal(SIGINT, interuptHandler);



    // Set up camera

    int ret = cam.initCamera();
    cam.configureStill(camWidth, camHeight, formats::RGB888, 1, Orientation::Rotate180);
    ControlList controls_;
    int64_t frame_time = 1000000 / 20;
	controls_.set(controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>({ frame_time, frame_time })); // Set frame rate
    controls_.set(controls::Brightness, 0.2); // Adjust the brightness of the output images, in the range -1.0 to 1.0
    controls_.set(controls::Contrast, 1.6); // Adjust the contrast of the output image, where 1.0 = normal contrast
    controls_.set(controls::ExposureTime, 20000);
    cam.set(controls_);

    if (ret) {
        cam.closeCamera();
        printf("Camera not working!\n");
        return -1;
    }

    bool flag;
    LibcameraOutData frameData;
    cam.startCamera();
    cam.VideoStream(&camWidth, &camHeight, &camStride);






    // Set up Raspberry Pi Pico I2C

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






    // Set up Lidar

    lidarController::LidarController lidar;
    if (!lidar.initialize() || !lidar.startScanning()) {
        return -1;
    }



    cv::namedWindow("LIDAR Hough Lines", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("LIDAR Hough Lines", onMouse);




    printf("Press Any Key to Start\n");  
    getchar();

    bno055_accel_float_t initialAccelData;
    bno055_euler_float_t initialEulerData;
    i2c_master_read_bno055_accel_and_euler(fd, &initialAccelData, &initialEulerData);

    int64 start = cv::getTickCount();
    while (isRunning) {
        flag = cam.readFrame(&frameData);
        if (!flag) {
            continue;
        }

        cv::Mat cameraImage(camHeight, camWidth, CV_8UC3, frameData.imageData, camStride);
        cam.returnFrameBuffer(frameData);


        uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];

        memcpy(movement, &motorPercent, sizeof(motorPercent));
        memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
        i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));


        bno055_accel_float_t accelData;
        bno055_euler_float_t eulerData;
        i2c_master_read_bno055_accel_and_euler(fd, &accelData, &eulerData);

        i2c_master_read_logs(fd, logs);
        i2c_master_print_logs(logs, sizeof(logs));




        auto lidarScanData = lidar.getScanData();
        // lidar.printScanData(lidarScanData);

        cv::Mat binaryImage = lidarDataToImage(lidarScanData, LIDAR_WIDTH, LIDAR_HEIGHT, LIDAR_SCALE);
        cv::Mat outputImage = cv::Mat::zeros(LIDAR_HEIGHT, LIDAR_WIDTH, CV_8UC3);
        cv::cvtColor(binaryImage, outputImage, cv::COLOR_GRAY2BGR);



        auto angle = fmod(eulerData.h - initialEulerData.h + 360.0f, 360.0f);



        auto lines = detectLines(binaryImage);
        auto combinedLines = combineAlignedLines(lines);
        auto wallDirections = analyzeWallDirection(combinedLines, angle, CENTER);

        drawAllLines(outputImage, combinedLines, wallDirections);




        Direction direction = NORTH;
        if (angle >= 337.5 || angle < 22.5) direction = NORTH;
        else if (angle >= 22.5 && angle < 112.5) direction = EAST;
        else if (angle >= 112.5 && angle < 202.5) direction = SOUTH;
        else if (angle >= 202.5 && angle < 292.5) direction = WEST;




        auto trafficLightPoints = detectTrafficLight(binaryImage, combinedLines, wallDirections, COUNTER_CLOCKWISE, direction);




        for (const auto point : trafficLightPoints) {
            cv::circle(outputImage, point, 5, cv::Scalar(255, 120, 255), cv::FILLED);
        }

        cv::circle(outputImage, CENTER, 10, cv::Scalar(0, 120, 255), cv::FILLED);

        cv::Mat filteredCameraImage = filterAllColors(cameraImage);
        cv::imshow("LIDAR Hough Lines", filteredCameraImage);






        // Check if a mouse click was detected
        if (clickPoint.x != -1 && clickPoint.y != -1) {
            // Get the pixel color at the clicked point (in BGR format)
            cv::Vec3b pixelColor = cameraImage.at<cv::Vec3b>(clickPoint);

            // Convert the BGR color to HSV
            cv::Mat hsvImage;
            cv::cvtColor(cameraImage, hsvImage, cv::COLOR_BGR2HSV);
            cv::Vec3b hsvColor = hsvImage.at<cv::Vec3b>(clickPoint);

            // Print the HSV values
            std::cout << "HSV of clicked pixel (" << clickPoint.x << ", " << clickPoint.y << "): "
                      << "H = " << (int)hsvColor[0] << ", "
                      << "S = " << (int)hsvColor[1] << ", "
                      << "V = " << (int)hsvColor[2] << std::endl;

            // Reset the clickPoint
            clickPoint = cv::Point(-1, -1);
        }









        char key = cv::waitKey(1);
        if (key == 'q') {
            break;
        }

        int64 end = cv::getTickCount();
        double duration = (end - start) / cv::getTickFrequency();
        double fps = 1.0 / duration;
        // printf("FPS: %.3f, # of lines: %d\n", fps, combinedLines.size());

        start = cv::getTickCount();
    }

    cam.stopCamera();
    cam.closeCamera();

    lidar.shutdown();
    cv::destroyAllWindows();

    return 0;
}
