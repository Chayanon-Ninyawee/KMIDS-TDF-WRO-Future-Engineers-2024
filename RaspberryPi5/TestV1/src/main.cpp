#include <iostream>
#include <vector>
#include <csignal>
#include <chrono>
#include <thread>

#include "rplidar.h"

using namespace rp::standalone::rplidar;

bool isRunning = true;

void interuptHandler(int signum)
{
  std::cout << "\nInterrupt signal (" << signum << ") received. Stopping the scan." << std::endl;
  isRunning = false;
}

int main(int argc, char *argv[])
{
  signal(SIGINT, interuptHandler);

  RPlidarDriver *rplidarDriver = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
  if (!rplidarDriver)
  {
    std::cerr << "Failed to create RPLidar driver." << std::endl;
    return -1;
  }

  u_result result = rplidarDriver->connect("/dev/ttyAMA0", 460800);
  if (IS_FAIL(result))
  {
    std::cerr << "Failed to connect to the RPLidar." << std::endl;
    RPlidarDriver::DisposeDriver(rplidarDriver);
    return -1;
  }

  rplidar_response_device_info_t deviceInfo;
  result = rplidarDriver->getDeviceInfo(deviceInfo);
  if (IS_OK(result))
  {
    std::cout << "RPLidar Device Info:" << std::endl;
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

  rplidarDriver->startMotor();

  std::vector<LidarScanMode> scanModes;
  rplidarDriver->getAllSupportedScanModes(scanModes);
  if (scanModes.empty())
  {
    std::cerr << "No supported scan modes available." << std::endl;
    rplidarDriver->stopMotor();
    RPlidarDriver::DisposeDriver(rplidarDriver);
    return -1;
  }

  std::cout << "Available scan modes:" << std::endl;
  for (const auto &mode : scanModes)
  {
    std::cout << " - Mode ID: " << mode.id << ", us per Sample: " << mode.us_per_sample << ", Max Distance: " << mode.max_distance << "m" << std::endl;
  }

  result = rplidarDriver->startScanExpress(false, scanModes[0].id);
  if (IS_FAIL(result))
  {
    std::cerr << "Failed to start scan." << std::endl;
    rplidarDriver->stopMotor();
    RPlidarDriver::DisposeDriver(rplidarDriver);
    return -1;
  }

  rplidar_response_measurement_node_hq_t nodes[8192];
  size_t nodeCount = sizeof(nodes)/sizeof(rplidar_response_measurement_node_hq_t);


  auto startTime = std::chrono::high_resolution_clock::now();

  while (isRunning)
  {
    result = rplidarDriver->grabScanDataHq(nodes, nodeCount, 0);
    if (IS_FAIL(result))
    {
      // std::cerr << "Failed to grab scan data." << std::endl;
      continue;
    }

    rplidarDriver->ascendScanData(nodes, nodeCount);
    for (size_t i = 0; i < nodeCount; ++i)
    {
      float angle = nodes[i].angle_z_q14 * 90.f / (1 << 14);
      float distance = nodes[i].dist_mm_q2 / 1000.f / (1 << 2);
      std::cout << "Angle: " << angle << " Dist: " << distance << " m" << nodeCount << std::endl;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsedTime = endTime - startTime;
    double refreshRate = 1 / elapsedTime.count(); // scans per second
    std::cout << " Node Count: " << nodeCount << " Refresh Rate: " << refreshRate << " scans/second" << std::endl;

    startTime = endTime;
  }

  rplidarDriver->stop();
  rplidarDriver->stopMotor();
  rplidarDriver->disconnect();
  RPlidarDriver::DisposeDriver(rplidarDriver);

  return 0;
}