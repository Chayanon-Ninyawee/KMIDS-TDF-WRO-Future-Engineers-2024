#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <EEPROM.h>
#include <Servo.h>
#include <CytronMotorDriver.h>
#include <VL53L0X.h>

VL53L0X front;
#define FRONT_PIN 4
#define FRONT_ADDR 0x30

VL53L0X back;
#define BACK_PIN 5
#define BACK_ADDR 0x31

VL53L0X left;
#define LEFT_PIN 6
#define LEFT_ADDR 0x32

VL53L0X right;
#define RIGHT_PIN 7
#define RIGHT_ADDR 0x33


// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
#define SAMPLERATE_DELAY_MS (10)

Servo steering_servo;
#define SERVO_MAX 165
#define SERVO_MIN 40

CytronMD motor(PWM_PWM, 2, 3);

float front_ultrasonic_distance = 0, back_ultrasonic_distance = 0, left_ultrasonic_distance = 0, right_ultrasonic_distance = 0;

float power_percent = 0, steering_percent = 0;
float last_power_percent = power_percent, last_steering_percent = steering_percent;

void setup() {
  pinMode(FRONT_PIN, OUTPUT);
  pinMode(BACK_PIN, OUTPUT);
  pinMode(LEFT_PIN, OUTPUT);
  pinMode(RIGHT_PIN, OUTPUT);

  digitalWrite(FRONT_PIN, LOW);
  digitalWrite(BACK_PIN, LOW);
  digitalWrite(LEFT_PIN, LOW);
  digitalWrite(RIGHT_PIN, LOW);

  Serial.begin(115200);

  delay(3000); // Booting up

  /* Initialise the sensor */
  if (!bno.begin())
  {
      Serial.println(F("There was a problem detecting the BNO055 ... check your connections"));
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
      Serial.println(F("No Calibration Data for this sensor exists in EEPROM"));
      delay(500);
  }
  else
  {
      eeAddress += sizeof(long);
      EEPROM.get(eeAddress, calibrationData);

      bno.setSensorOffsets(calibrationData);

      Serial.println(F("Calibration data loaded into BNO055"));
      foundCalib = true;
  }

  delay(1000);

  /* Crystal must be configured AFTER loading calibration data into BNO055. */
  bno.setExtCrystalUse(true);

  sensors_event_t event;
  bno.getEvent(&event);
  /* always recal the mag as It goes out of calibration very often */
  if (foundCalib){
      Serial.println(F("Move sensor slightly to calibrate magnetometers"));
      while (!bno.isFullyCalibrated())
      {
          bno.getEvent(&event);
          delay(SAMPLERATE_DELAY_MS);
      }
  }
  else
  {
      Serial.println(F("Please Calibrate Sensor: "));
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
  Serial.println(F("Data stored to EEPROM."));

  delay(500);

  steering_servo.attach(9);
  motor.setSpeed(0);

  steering_servo.write(SERVO_MIN);
  delay(5000);
  steering_servo.write(SERVO_MAX);
  delay(5000);
  steering_servo.write((SERVO_MAX+SERVO_MIN)/2);
  delay(1000);


  bool is_running = true;

  delay(10);
  digitalWrite(FRONT_PIN, HIGH);
  delay(10);
  front.setAddress(FRONT_ADDR);
  if(!front.init()) {
    Serial.println(F("Failed to boot front"));
    is_running = false;
  } else {
    Serial.println(F("Booted front"));

    front.setSignalRateLimit(0.1);
    front.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    front.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

    front.setMeasurementTimingBudget(52000);
  }

  delay(10);
  digitalWrite(BACK_PIN, HIGH);
  delay(10);
  back.setAddress(BACK_ADDR);
  if(!back.init()) {
    Serial.println(F("Failed to boot back"));
    is_running = false;
  } else {
    Serial.println(F("Booted back"));

    back.setSignalRateLimit(0.1);
    back.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    back.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
    
    back.setMeasurementTimingBudget(52000);
  }

  delay(10);
  digitalWrite(LEFT_PIN, HIGH);
  delay(10);
  left.setAddress(LEFT_ADDR);
  if(!left.init()) {
    Serial.println(F("Failed to boot left"));
    is_running = false;
  } else {
    Serial.println(F("Booted left"));

    left.setSignalRateLimit(0.1);
    left.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    left.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

    left.setMeasurementTimingBudget(52000);
  }

  delay(10);
  digitalWrite(RIGHT_PIN, HIGH);
  delay(10);
  right.setAddress(RIGHT_ADDR);
  if(!right.init()) {
    Serial.println(F("Failed to boot right"));
    is_running = false;
  } else {
    Serial.println(F("Booted right"));

    right.setSignalRateLimit(0.1);
    right.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    right.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);

    right.setMeasurementTimingBudget(52000);
  }

  if (!is_running) {
    while(1);    
  }

  front.startContinuous();
  back.startContinuous();
  left.startContinuous();
  right.startContinuous();
}

long last_micros = 0;
void loop() {
  long current_micros = micros();
  long delay_micros_duration = 55000 - (current_micros - last_micros);
  last_micros = current_micros;

  if (delay_micros_duration > 0) {
    delayMicroseconds(delay_micros_duration);
  }

  uint16_t front_data = front.readRangeContinuousMillimeters();
  uint16_t back_data = back.readRangeContinuousMillimeters();
  uint16_t left_data = left.readRangeContinuousMillimeters();
  uint16_t right_data = right.readRangeContinuousMillimeters();

  if (front_data < 8190) {
    front_ultrasonic_distance = front_data / 1000.0;
  }
  if (back_data < 8190) {
    back_ultrasonic_distance = back_data / 1000.0;
  }
  if (left_data < 8190) {
    left_ultrasonic_distance = left_data / 1000.0;
  }
  if (right_data < 8190) {
    right_ultrasonic_distance = right_data / 1000.0;
  }

  // front_ultrasonic_distance = front_data / 1000.0;
  // back_ultrasonic_distance = back_data / 1000.0;
  // left_ultrasonic_distance = left_data / 1000.0;
  // right_ultrasonic_distance = right_data / 1000.0;
  
  sensors_event_t event;
  bno.getEvent(&event);

  float gyro_x = event.orientation.x;

  float data[] = {
    front_ultrasonic_distance,
    back_ultrasonic_distance,
    left_ultrasonic_distance,
    right_ultrasonic_distance,
    gyro_x
  };

  // Serial.print(front_ultrasonic_distance);
  // Serial.print(F(" "));
  // Serial.print(back_ultrasonic_distance);
  // Serial.print(F(" "));
  // Serial.print(left_ultrasonic_distance);
  // Serial.print(F(" "));
  // Serial.print(right_ultrasonic_distance);
  // Serial.print(F(" "));
  // Serial.print(gyro_x);
  // Serial.println();

  Serial.write((byte*)data, sizeof(data));
  Serial.flush();

  // if (front_data > 8191) {
  //   pinMode(FRONT_PIN, OUTPUT);
  //   digitalWrite(FRONT_PIN, LOW);
  //   delay(20);
  //   pinMode(FRONT_PIN, INPUT);
  //   delay(20);
  //   front.setAddress(FRONT_ADDR);
  //   front.init();
  //   front.startContinuous();
  // }
  // if (back_data > 8191) {
  //   pinMode(BACK_PIN, OUTPUT);
  //   digitalWrite(BACK_PIN, LOW);
  //   delay(20);
  //   pinMode(BACK_PIN, INPUT);
  //   delay(20);
  //   back.setAddress(BACK_ADDR);
  //   back.init();
  //   back.startContinuous();
  // }
  // if (left_data > 8191) {
  //   pinMode(LEFT_PIN, OUTPUT);
  //   digitalWrite(LEFT_PIN, LOW);
  //   delay(20);
  //   pinMode(LEFT_PIN, INPUT);
  //   delay(20);
  //   left.setAddress(LEFT_ADDR);
  //   left.init();
  //   left.startContinuous();
  // }
  // if (right_data > 8191) {
  //   pinMode(RIGHT_PIN, OUTPUT);
  //   digitalWrite(RIGHT_PIN, LOW);
  //   delay(20);
  //   pinMode(RIGHT_PIN, INPUT);
  //   delay(20);
  //   right.setAddress(RIGHT_ADDR);
  //   right.init();
  //   right.startContinuous();
  // }

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
}