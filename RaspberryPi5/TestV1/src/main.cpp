#include <cstdlib>
#include <csignal>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>

#define	LED_PIN	15
#define BUTTON_PIN 16


volatile bool isStarted = false;

void mainButtonInterrupt(void) {
  isStarted = true;
}


void cleanup() {
  pinMode(LED_PIN, INPUT);
  pullUpDnControl(LED_PIN, PUD_OFF);

  pinMode(BUTTON_PIN, INPUT);
  pullUpDnControl(BUTTON_PIN, PUD_OFF);
}

void exiting() {
  cleanup();
}

void exiting(int i) {
  exit(0);
}

int main(int argc, char * argv[]) {
  if (wiringPiSetup() < 0) {
    printf("Failed to initialize wiringPi: %s\n", strerror(errno));
    return 1;
  }

  pinMode(BUTTON_PIN, INPUT);
  pullUpDnControl(BUTTON_PIN, PUD_UP);

  if (wiringPiISR(BUTTON_PIN, INT_EDGE_FALLING, &mainButtonInterrupt) < 0) {
    printf("Failed to initialize ISR: %s\n", strerror(errno));
    return 1;
  }

  pinMode(LED_PIN, OUTPUT);


  signal(SIGINT, exiting);
  signal(SIGABRT, exiting);
  signal(SIGTERM, exiting);
  signal(SIGTSTP, exiting);
  atexit(exiting);
  

  printf("Waiting for button press...\n");
  while (!isStarted) {
    delay(50);
  }

  while (true) {
    digitalWrite (LED_PIN, HIGH);
    printf("LED ON!\n");
    delay(1000);
    digitalWrite (LED_PIN, LOW);
    printf("LED OFF!\n");
    delay(1000);
  }
  
  return 0;
}