#include <csignal>
#include "lidarController.h"

bool isRunning = true;

void interuptHandler(int signum)
{
  isRunning = false;
}

int main() {
  signal(SIGINT, interuptHandler);

  lidarController::LidarController lidar;
  
  if (!lidar.initialize()) {
    return -1;
  }

  if (!lidar.startScanning()) {
    return -1;
  }

  while (isRunning)
  {
    auto lidarScanData = lidar.getScanData();
    lidar.printScanData(lidarScanData);
  }

  lidar.shutdown();
  return 0;
}