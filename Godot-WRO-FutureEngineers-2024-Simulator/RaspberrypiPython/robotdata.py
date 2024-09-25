import serial
from picamera2 import Picamera2
from libcamera import controls
import numpy as np
import cv2
import struct
import time

class SerialCommunicator:
    def __init__(self, port, baud_rate=115200):
        """Initialize the serial communication."""
        self.ser = serial.Serial(port, baud_rate)
        self.baud_rate = baud_rate

    def send_floats(self, floats):
        """
        Send a list of floats over serial.
        
        Args:
            floats (list of float): List of floats to send.
        """
        # Pack the floats into bytes using struct
        data_to_send = struct.pack(f'{len(floats)}f', *floats)
        self.ser.write(data_to_send)

    def receive_floats(self, num_floats):
        """
        Receive a specified number of floats from serial.
        
        Args:
            num_floats (int): Number of floats to receive.

        Returns:
            list of float: Received floats, or None if not enough data.
        """
        num_bytes = num_floats * 4  # Each float is 4 bytes
        data = self.ser.read(num_bytes)
        floats = None
        try:
            floats = struct.unpack(f'{num_floats}f', data)
        except Exception as e:
            print(e)
        return floats

    def close(self):
        """Close the serial communication."""
        if self.ser.is_open:
            self.ser.close()
            print("Serial port closed.")

class RobotData:
    """
    A class to manage robot data, including image capture and sensor readings.

    Args:
        image_width (int): The width of the image to capture.
        image_height (int): The height of the image to capture.

    Attributes:
        image_width (int): The width of the image to capture.
        image_height (int): The height of the image to capture.
        _communicator (SerialCommunicator): An instance of SerialCommunicator for serial communication.
        _picam2 (Picamera2): An instance of Picamera2 for capturing images.
        _init_gyro (float): Initial gyro reading to normalize subsequent readings.
    """
    _RECEIVE_FLOAT_AMOUNT = 5

    def __init__(self, image_width: int, image_height: int):
        """Initialize the robot data, including serial communication and camera configuration."""
        self.image_width: int = image_width
        self.image_height: int = image_height

        self._communicator = SerialCommunicator('/dev/ttyAMA0', 115200)
        
        self._picam2 = Picamera2()

        config = self._picam2.create_preview_configuration(raw={"size":(2304, 1296)},main={"size": (self.image_width, self.image_height)})
        self._picam2.configure(config)

        self._picam2.set_controls({"AfMode": controls.AfModeEnum.Manual, "LensPosition": 2.0})
        self._picam2.set_controls({"AeEnable": False , "ExposureTime": 3000, "AnalogueGain": 6.2, "ColourGains": (1.9699761867523193, 1.9044568538665771)})
        self._picam2.set_controls({"AwbEnable": False, "Brightness": 0.1, "Contrast": 1.2})
        self._picam2.set_controls({"HdrMode": controls.HdrModeEnum.Off})

        self._picam2.start()

        self.re_init_serial()

    def re_init_serial(self) -> None:
        self._communicator.ser.reset_output_buffer()
        self._communicator.ser.reset_input_buffer()
        time.sleep(0.100)
        self._communicator.send_floats((0, 0))
        time.sleep(0.100)
        self._communicator.ser.reset_input_buffer()
        time.sleep(0.100)
        self._communicator.send_floats((0, 0))

        while True:
            received_floats = self._communicator.receive_floats(self._RECEIVE_FLOAT_AMOUNT)
            self._communicator.ser.reset_input_buffer()
            time.sleep(0.100)
            self._communicator.send_floats((0, 0))

            data = []
            if received_floats:
                for received_float in received_floats:
                    data.append(received_float)

                self._init_gyro = data[4]
                break
            else:
                print("Can't connect with Arduino via Serial!")

    def process_data(self) -> None:
        """
        Process incoming data from the robot's sensors and capture an image.

        This method receives float data from the serial connection, captures an image from the camera,
        and processes sensor readings, including ultrasonic sensor data and gyro data.
        """
        received_floats = self._communicator.receive_floats(self._RECEIVE_FLOAT_AMOUNT)
        self._communicator.ser.reset_input_buffer()

        data = []
        if received_floats:
            for received_float in received_floats:
                data.append(received_float)
            
            self._image = self._picam2.capture_array()
            self._image = cv2.rotate(self._image, cv2.ROTATE_180)
            self._image = cv2.cvtColor(self._image, cv2.COLOR_RGB2BGR)

            self._front_ultrasonic: float = data[0]
            self._back_ultrasonic: float = data[1]
            self._left_ultrasonic: float = data[2]
            self._right_ultrasonic: float = data[3]

            self._gyro = round(data[4] - self._init_gyro) % 360
        else:
            print("Can't connect with Arduino via Serial!")

    def close(self) -> None:
        self._communicator.send_floats((0, 0))
        self._communicator.ser.flush()
        time.sleep(0.1)
        self._communicator.close()
        time.sleep(0.5)
        self._picam2.close()

    def get_image(self) -> np.ndarray:
        """
        Returns the captured image data.

        Note:
            The returned image is a reference to the internal image array. Modifying
            this image will affect the internal state of the RobotData instance.
            If you need to modify the image, create a copy first using `image.copy()`.

        Returns:
            np.ndarray: The captured image data as a NumPy array.
        """
        return self._image

    def get_image_copy(self) -> np.ndarray:
        """
        Returns a copy of the captured image data.

        Returns:
            np.ndarray: A copy of the captured image data as a NumPy array.
        """
        return self._image.copy()
    
    def get_ultrasonic_data(self) -> tuple[float, float, float, float]:
        """
        Retrieve ultrasonic sensor data for the front, back, left, and right directions.

        Returns:
            tuple: A tuple containing four float values representing the ultrasonic sensor readings:
                - front ultrasonic reading
                - back ultrasonic reading
                - left ultrasonic reading
                - right ultrasonic reading
        """
        return self._front_ultrasonic, self._back_ultrasonic, self._left_ultrasonic, self._right_ultrasonic
    
    def get_gyro_data(self) -> float:
        """
        Retrieve gyro sensor data.

        Returns:
            float: The gyro sensor reading.
        """
        return self._gyro
    
    def send_data(self, speed_target: float, steering_percent: float) -> None:
        """
        Send control data to the robot via serial communication.

        Args:
            speed_target (float): The target speed value.
            steering_percent (float): The steering percentage value.
        """
        self._communicator.send_floats((steering_percent, speed_target))
        self._communicator.ser.flush()
