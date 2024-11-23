#include <SDL2/SDL.h>

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <thread>

#include "utils/i2c_master.h"


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

  
  int fd = i2c_master_init(PICO_ADDRESS);

  i2c_master_send_command(fd, Command::RESTART);
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  i2c_master_send_command(fd, Command::SKIP_CALIB);

  uint8_t status[i2c_slave_mem_addr::STATUS_SIZE] = {0};
  uint8_t logs[i2c_slave_mem_addr::LOGS_BUFFER_SIZE] = {0};
  while (not (status[0] & (1 << 1))) {
    i2c_master_read_data(fd, i2c_slave_mem_addr::STATUS_ADDR, status, sizeof(status));

    i2c_master_read_logs(fd, logs);
    i2c_master_print_logs(logs, sizeof(logs));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  i2c_master_send_command(fd, Command::NO_COMMAND);
  std::this_thread::sleep_for(std::chrono::milliseconds(20));

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
    uint8_t movement[sizeof(motorPercent) + sizeof(steeringPercent)];
  
    memcpy(movement, &motorPercent, sizeof(motorPercent));
    memcpy(movement + sizeof(motorPercent), &steeringPercent, sizeof(steeringPercent));
    i2c_master_send_data(fd, i2c_slave_mem_addr::MOVEMENT_INFO_ADDR, movement, sizeof(movement));

    i2c_master_read_logs(fd, logs);
    i2c_master_print_logs(logs, sizeof(logs));


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