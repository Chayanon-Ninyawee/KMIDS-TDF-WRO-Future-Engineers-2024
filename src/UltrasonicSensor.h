#ifndef ULTRASONICSENSOR_H
#define ULTRASONICSENSOR_H

#include <Arduino.h>

class UltrasonicSensor {
public:
    UltrasonicSensor(int trigPin, int echoPin);
    void begin();
    float getDistance();
    void setSpeedOfSound(float speed);

private:
    int _trigPin;
    int _echoPin;
    float _speedOfSound; // Speed of sound in cm/us
};

#endif
