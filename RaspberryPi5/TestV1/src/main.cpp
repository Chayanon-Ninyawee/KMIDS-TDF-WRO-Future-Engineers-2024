#include <iostream>
#include <csignal>
#include <chrono>

#include "sl_lidar.h"
#include "sl_lidar_driver.h"

using namespace sl;

// Constant
const char* const SERIAL_PORT = "/dev/ttyAMA0";          // Serial port path
const int LIDAR_BAUD_RATE = 460800;                    // Baud rate for LIDAR communication

bool isRunning = true;

void interuptHandler(int signum)
{
  std::cout << "\nInterrupt signal (" << signum << ") received. Stopping the scan." << std::endl;
  isRunning = false;
}

int main(int argc, char *argv[])
{
  signal(SIGINT, interuptHandler);

  ILidarDriver *lidarDriver = *createLidarDriver();
  if (!lidarDriver)
  {
    std::cerr << "Failed to create SLAMTEC LIDAR driver." << std::endl;
    return -1;
  }

  IChannel* serialChannel = *createSerialPortChannel(SERIAL_PORT, LIDAR_BAUD_RATE);
  if (!serialChannel)
  {
    std::cerr << "Failed to create serial port channel." << std::endl;

    delete lidarDriver;
    lidarDriver = NULL;

    return -1;
  }

  sl_result result = lidarDriver->connect(serialChannel);
  if (SL_IS_FAIL(result))
  {
    std::cerr << "Failed to connect to the LIDAR." << std::endl;

    delete serialChannel;
    serialChannel = NULL;

    delete lidarDriver;
    lidarDriver = NULL;

    return -1;
  }

  sl_lidar_response_device_info_t deviceInfo;
  result = lidarDriver->getDeviceInfo(deviceInfo);
  if (SL_IS_OK(result))
  {
    std::cout << "LIDAR Device Info:" << std::endl;
    std::cout << " - Model: " << static_cast<int>(deviceInfo.model) << std::endl;
    std::cout << " - Firmware Version: " << (deviceInfo.firmware_version >> 8) << "." << (deviceInfo.firmware_version & 0xFF) << std::endl;
    std::cout << " - Hardware Version: " << static_cast<int>(deviceInfo.hardware_version) << std::endl;
    std::cout << " - Serial Number: ";
    for (int i = 0; i < 16; ++i)
    {
      printf("%02X", deviceInfo.serialnum[i]);
    }
    std::cout << std::endl;
  }
  else
  {
    std::cerr << "Failed to retrieve device information." << std::endl;
  }

  lidarDriver->setMotorSpeed();

  result = lidarDriver->startScan(0,1);
  if (SL_IS_FAIL(result))
  {
    std::cerr << "Failed to start scan." << std::endl;
    lidarDriver->setMotorSpeed(0);

    delete serialChannel;
    serialChannel = NULL;

    delete lidarDriver;
    lidarDriver = NULL;

    return -1;
  }


  auto startTime = std::chrono::high_resolution_clock::now();

  while (isRunning)
  {
    sl_lidar_response_measurement_node_hq_t nodes[8192];
    size_t nodeCount = sizeof(nodes) / sizeof(sl_lidar_response_measurement_node_hq_t);

    result = lidarDriver->grabScanDataHq(nodes, nodeCount);
    if (SL_IS_FAIL(result)) continue;

    lidarDriver->ascendScanData(nodes, nodeCount);
    for (size_t i = 0; i < nodeCount; ++i)
    {
      float angle = nodes[i].angle_z_q14 * 90.f / (1 << 14);
      float distance = nodes[i].dist_mm_q2 / 1000.f / (1 << 2);
      printf("Angle: %.3f\tDistance: %.3f m\n", angle, distance);
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = endTime - startTime;
    double refreshRate = 1 / elapsedTime.count(); // scans per second
    std::cout << " Node Count: " << nodeCount << " Refresh Rate: " << refreshRate << " scans/second" << std::endl;

    startTime = endTime;
  }

  lidarDriver->stop();
  lidarDriver->setMotorSpeed(0);
  lidarDriver->disconnect();

  delete serialChannel;
  serialChannel = NULL;

  delete lidarDriver;
  lidarDriver = NULL;

  return 0;
}