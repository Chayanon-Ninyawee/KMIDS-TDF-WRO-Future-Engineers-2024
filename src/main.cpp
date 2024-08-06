#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <EEPROM.h>
#include <Servo.h>
#include <CytronMotorDriver.h>

#include <UltrasonicSensor.h>

UltrasonicSensor front_ultrasonic(6, 7);
UltrasonicSensor left_ultrasonic(10, 11);
UltrasonicSensor right_ultrasonic(4, 5);

/* Set the delay between fresh samples */
#define SAMPLERATE_DELAY_MS (10)

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
// Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

#define RPI_ADDR 1


Servo steering_servo;

#define SERVO_MAX 165
#define SERVO_MIN 40

CytronMD motor(PWM_PWM, 2, 3);


float front_ultrasonic_distance, left_ultrasonic_distance, right_ultrasonic_distance;
float gyro_x;

float power_percent = 0, steering_percent = 0;
float last_power_percent = power_percent, last_steering_percent = steering_percent;

void setup() {
  Serial.begin(9600);
  Serial.begin(9600);

  delay(3000); // Booting up

  front_ultrasonic.begin();
  left_ultrasonic.begin();
  right_ultrasonic.begin();

  /* Initialise the sensor */
    if (!bno.begin())
    {
        /* There was a problem detecting the BNO055 ... check your connections */
        // TODO: Send Error
        while (1);
    }

    int eeAddress = 0;
    long bnoID;
    bool foundCalib = false;

    EEPROM.get(eeAddress, bnoID);

    adafruit_bno055_offsets_t calibrationData;
    sensor_t sensor;

    /*
    *  Look for the sensor's unique ID at the beginning oF EEPROM.
    *  This isn't foolproof, but it's better than nothing.
    */
    bno.getSensor(&sensor);
    if (bnoID != sensor.sensor_id)
    {
        // No Calibration Data for this sensor exists in EEPROM
        delay(500);
    }
    else
    {
        eeAddress += sizeof(long);
        EEPROM.get(eeAddress, calibrationData);

        bno.setSensorOffsets(calibrationData);

        // Calibration data loaded into BNO055
        foundCalib = true;
    }

    delay(1000);

    /* Crystal must be configured AFTER loading calibration data into BNO055. */
    bno.setExtCrystalUse(true);

    sensors_event_t event;
    bno.getEvent(&event);
    /* always recal the mag as It goes out of calibration very often */
    if (foundCalib){
        // Move sensor slightly to calibrate magnetometers
        while (!bno.isFullyCalibrated())
        {
            bno.getEvent(&event);
            delay(SAMPLERATE_DELAY_MS);
        }
    }
    else
    {
        // "Please Calibrate Sensor: 
        while (!bno.isFullyCalibrated())
        {
            bno.getEvent(&event);

            /* Wait the specified delay before requesting new data */
            delay(SAMPLERATE_DELAY_MS);
        }
    }

    adafruit_bno055_offsets_t newCalib;
    bno.getSensorOffsets(newCalib);

    eeAddress = 0;
    bno.getSensor(&sensor);
    bnoID = sensor.sensor_id;

    EEPROM.put(eeAddress, bnoID);

    eeAddress += sizeof(long);
    EEPROM.put(eeAddress, newCalib);
    // "Data stored to EEPROM.

    delay(500);

    steering_servo.attach(9);
    motor.setSpeed(0);

    steering_servo.write(SERVO_MIN);
    delay(5000);
    steering_servo.write(SERVO_MAX);
    delay(5000);
    steering_servo.write((SERVO_MAX+SERVO_MIN)/2);
    delay(1000);
}

void loop() {
  front_ultrasonic_distance = front_ultrasonic.getDistance();
  left_ultrasonic_distance = left_ultrasonic.getDistance();
  right_ultrasonic_distance = right_ultrasonic.getDistance();

  /* Get a new sensor event */
  sensors_event_t event;
  bno.getEvent(&event);

  gyro_x = event.orientation.x;

  Serial.write((byte*)&front_ultrasonic_distance, sizeof(front_ultrasonic_distance));
  Serial.write((byte*)&left_ultrasonic_distance, sizeof(left_ultrasonic_distance));
  Serial.write((byte*)&right_ultrasonic_distance, sizeof(right_ultrasonic_distance));

  Serial.write((byte*)&gyro_x, sizeof(gyro_x));

  Serial.flush();

  float recieveData[2];
  byte recieveBuffer[sizeof(recieveData)];

  int received_bytes_num = Serial.readBytes((char*)recieveBuffer, (int)sizeof(recieveBuffer));
  if (received_bytes_num == (int)sizeof(recieveBuffer)) {
    memcpy(recieveData, recieveBuffer, sizeof(recieveData));

    steering_percent = recieveData[0];
    power_percent = recieveData[1];
  }

  if (steering_percent != last_steering_percent) {
    steering_servo.write(map(round(steering_percent*10000), -10000, 10000, SERVO_MIN, SERVO_MAX));
    last_steering_percent = steering_percent;
  }

  if (power_percent != last_power_percent) {
    motor.setSpeed(round(255*power_percent));
    last_power_percent = power_percent;
  }

  /* Wait the specified delay before requesting new data */
  delay(SAMPLERATE_DELAY_MS);
}