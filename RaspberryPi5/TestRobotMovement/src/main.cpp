#include <SDL2/SDL.h>
#include <wiringPiI2C.h>

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>


const uint8_t PICO_ADDRESS = 0x39;


// Shared state (if needed for debugging)
float motorPercent = 0.0f;
float steeringPercent = 0.0f;
float motorPercentSetting = 1.0f;

bool isRunning = true;

// Signal handler for graceful shutdown
void interruptHandler(int signum) {
  isRunning = false;
}

// Key press handling
void handleKeyDown(SDL_Keycode key) {
  switch (key) {
    case SDLK_w:
      motorPercent = motorPercentSetting;
      break;  // Forward
    case SDLK_s:
      motorPercent = -motorPercentSetting;
      break;  // Backward
    case SDLK_a:
      steeringPercent = -0.55f;
      break;  // Left
    case SDLK_d:
      steeringPercent = 0.65f;
      break;  // Right
    case SDLK_1:
      motorPercentSetting = 0.1f;
      break;
    case SDLK_2:
      motorPercentSetting = 0.2f;
      break;
    case SDLK_3:
      motorPercentSetting = 0.3f;
      break;
    case SDLK_4:
      motorPercentSetting = 0.4f;
      break;
    case SDLK_5:
      motorPercentSetting = 0.5f;
      break;
    case SDLK_6:
      motorPercentSetting = 0.6f;
      break;
    case SDLK_7:
      motorPercentSetting = 0.7f;
      break;
    case SDLK_8:
      motorPercentSetting = 0.8f;
      break;
    case SDLK_9:
      motorPercentSetting = 0.9f;
      break;
    case SDLK_0:
      motorPercentSetting = 1.0f;
      break;
  }
}

void handleKeyUp(SDL_Keycode key) {
  switch (key) {
    case SDLK_w:
    case SDLK_s:
      motorPercent = 0.0f;
      break;  // Stop moving
    case SDLK_a:
    case SDLK_d:
      steeringPercent = 0.05f;
      break;  // Center steering
  }
}




void print_logs(uint8_t *logs, size_t len) {
  if (logs == NULL) {
    return; // Handle null pointer gracefully
  }

  printf("Processed logs:\n");
  for (int i = 0; i < len; i++) {
    if (logs[i] == 0xFF) {
      // continue;
      break; // Stop at the first occurrence of 0xFF
    }

    printf("%c", logs[i]); // Print the character directly
  }
  printf("\n");
}


int main() {
  signal(SIGINT, interruptHandler);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Window* window = SDL_CreateWindow("SDL Input",
                                        SDL_WINDOWPOS_CENTERED,
                                        SDL_WINDOWPOS_CENTERED,
                                        640, 480, SDL_WINDOW_SHOWN);
  if (!window) {
    std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_Event event;

  // Setup I2C communication
  int fd = wiringPiI2CSetup(PICO_ADDRESS);
  if (fd == -1) {
    printf("Failed to initialize I2C communication.\n");
    return -1;
  }
  printf("I2C communication successfully initialized.\n");

  uint8_t restart_cmd[2] = {0, 0x01};
  wiringPiI2CRawWrite(fd, restart_cmd, sizeof(restart_cmd));
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  uint8_t skip_calib_cmd[2] = {0, 0x04};
  wiringPiI2CRawWrite(fd, skip_calib_cmd, sizeof(skip_calib_cmd));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  uint8_t status[1] = {0};
  uint8_t logs[256] = {0};
  while (not (status[0] & (1 << 1))) {
    wiringPiI2CReadBlockData(fd, 1, status, sizeof(status));
    printf("%x\n", status[0]);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    wiringPiI2CWrite(fd, 56);
    read(fd, logs, sizeof(logs));
    print_logs(logs, sizeof(logs));
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
  }
  printf("%x\n", status[0]);

  uint8_t noop_cmd[2] = {0, 0x00};
  wiringPiI2CRawWrite(fd, noop_cmd, sizeof(noop_cmd));
  std::this_thread::sleep_for(std::chrono::milliseconds(10));


  while (isRunning) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        isRunning = false;
      } else if (event.type == SDL_KEYDOWN) {
        handleKeyDown(event.key.keysym.sym);
      } else if (event.type == SDL_KEYUP) {
        handleKeyUp(event.key.keysym.sym);
      }
      SDL_Delay(10);
    }

    // Send movement data via I2C
    uint8_t movement[1 + sizeof(motorPercent) + sizeof(steeringPercent)];

    movement[0] = 248;
  
    memcpy(&movement[1], &motorPercent, sizeof(motorPercent));
    memcpy(&movement[1] + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
    wiringPiI2CRawWrite(fd, movement, sizeof(movement));

    printf("%.2f, %.2f\n", motorPercent, steeringPercent);


    // Render window
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Delay(10);  // Control loop speed
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}