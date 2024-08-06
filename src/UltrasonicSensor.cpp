#include "UltrasonicSensor.h"

UltrasonicSensor::UltrasonicSensor(int trigPin, int echoPin)
    : _trigPin(trigPin), _echoPin(echoPin), _speedOfSound(0.034) {} // Default speed of sound in cm/us

void UltrasonicSensor::begin() {
    pinMode(_trigPin, OUTPUT);
    pinMode(_echoPin, INPUT);
}

float UltrasonicSensor::getDistance() {
    long duration;
    float distance;

    // Clear the trigPin by setting it LOW
    digitalWrite(_trigPin, LOW);
    delayMicroseconds(2);

    // Trigger the sensor by setting the trigPin high for 10 microseconds
    digitalWrite(_trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(_trigPin, LOW);

    // Read the echoPin, which will be HIGH for the duration of the sound wave return
    duration = pulseIn(_echoPin, HIGH);

    // Calculate the distance (in m) based on the speed of sound
    distance = duration * _speedOfSound / 2.0 / 100.0;

    return distance;
}

void UltrasonicSensor::setSpeedOfSound(float speed) {
    _speedOfSound = speed;
}
