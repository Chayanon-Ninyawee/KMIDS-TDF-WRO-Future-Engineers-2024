# TODO: Test this

import cv2
import numpy as np
from smbus2 import SMBus
import time

class RobotData:
    def __init__(self, camera_port=0, arduino_address=0x04, bno055_address=0x28, i2c_bus=1):
        self._camera = cv2.VideoCapture(camera_port)
        self._image = None
        self._front_ultrasonic = 0.0
        self._back_ultrasonic = 0.0
        self._left_ultrasonic = 0.0
        self._right_ultrasonic = 0.0
        self._gyro = 0.0
        self._arduino_address = arduino_address
        self._bno055_address = bno055_address
        self._i2c_bus = i2c_bus
        self._bus = SMBus(i2c_bus)

    def _capture_image(self):
        ret, frame = self._camera.read()
        if ret:
            self._image = frame

    def get_image(self) -> np.ndarray:
        self._capture_image()
        return self._image

    def get_image_copy(self) -> np.ndarray:
        self._capture_image()
        return self._image.copy()

    def _read_ultrasonic_data(self):
        try:
            # Assuming Arduino sends 4 floats each of 4 bytes (16 bytes total)
            data = self._bus.read_i2c_block_data(self._arduino_address, 0, 16)
            self._front_ultrasonic = np.frombuffer(bytearray(data[0:4]), dtype=np.float32)[0]
            self._back_ultrasonic = np.frombuffer(bytearray(data[4:8]), dtype=np.float32)[0]
            self._left_ultrasonic = np.frombuffer(bytearray(data[8:12]), dtype=np.float32)[0]
            self._right_ultrasonic = np.frombuffer(bytearray(data[12:16]), dtype=np.float32)[0]
        except Exception as e:
            print(f"Error reading ultrasonic data: {e}")

    def get_ultrasonic_data(self) -> tuple[float, float, float, float]:
        self._read_ultrasonic_data()
        return self._front_ultrasonic, self._back_ultrasonic, self._left_ultrasonic, self._right_ultrasonic

    def _read_gyro_data(self):
        try:
            # Assuming the BNO055 chip's register for gyro data is 0x1A (consult the datasheet for accurate registers)
            data = self._bus.read_i2c_block_data(self._bno055_address, 0x1A, 6)
            x = np.frombuffer(bytearray(data[0:2]), dtype=np.int16)[0]
            y = np.frombuffer(bytearray(data[2:4]), dtype=np.int16)[0]
            z = np.frombuffer(bytearray(data[4:6]), dtype=np.int16)[0]
            self._gyro = (x, y, z)
        except Exception as e:
            print(f"Error reading gyro data: {e}")

    def get_gyro_data(self) -> tuple[int, int, int]:
        self._read_gyro_data()
        return self._gyro

    def __del__(self):
        self._camera.release()
        self._bus.close()