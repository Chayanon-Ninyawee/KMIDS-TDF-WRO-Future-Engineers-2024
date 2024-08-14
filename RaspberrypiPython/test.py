import cv2
import numpy as np
import math
import serial
import struct
import time

from openchallenge import process_data_open

from picamera2 import Picamera2
from libcamera import controls

# Initialize the Picamera2
picam2 = Picamera2()

# Configure the camera
config = picam2.create_preview_configuration(main={"size": (854, 480)})
picam2.configure(config)

picam2.set_controls({"AfMode":controls.AfModeEnum.Manual, "LensPosition": 0.5})
picam2.set_controls({"AwbEnable": False, "Brightness": 0.1})
picam2.set_controls({"HdrMode": controls.HdrModeEnum.Off})

# Start the camera
picam2.start()

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

communicator = SerialCommunicator('/dev/ttyAMA0', 9600)


def average_x_coordinate(mask):
    # Find non-zero coordinates
    coordinates = np.column_stack(np.where(mask > 0))
    if coordinates.size == 0:
        return None, None
    average_x = np.mean(coordinates[:, 1])
    return average_x, coordinates.size

def average_y_coordinate(mask):
    # Find non-zero coordinates
    coordinates = np.column_stack(np.where(mask > 0))
    if coordinates.size == 0:
        return None, None
    average_y = np.mean(coordinates[:, 0])
    return int(average_y), coordinates.size

def process_image(image: cv2.typing.MatLike):
    """
    Process the input image to generate masks and coordinates for blue lines, orange lines, red lights, and green lights.

    Args:
        image (cv2.typing.MatLike): The input image to be processed.

    Returns:
        tuple: A tuple containing four tuples with the mask, coordinate, and size for each of the blue line, orange line, red light, and green light.
            - blue_line_info (tuple): (mask, y-coordinate, size) for the blue line.
            - orange_line_info (tuple): (mask, y-coordinate, size) for the orange line.
            - red_light_info (tuple): (mask, x-coordinate, size) for the red light.
            - green_light_info (tuple): (mask, x-coordinate, size) for the green light.
    """
    # Threshold of blue line in HSV space
    # Measured blue line HSV: 109, 218, 173
    lower_blue_line = np.array([99, 178, 133]) 
    upper_blue_line = np.array([119, 248, 213])

    # Threshold of orange line in HSV space
    # Measured orange line HSV: 15, 163, 204
    lower_orange_line = np.array([5, 23, 114]) 
    upper_orange_line = np.array([25, 213, 244])

    # Threshold of red traffic light in HSV space
    # Measured red traffic light HSV: 177, 213, 238
    lower_red_light = np.array([172, 203, 70]) 
    upper_red_light = np.array([182, 223, 255])

    # Threshold of green traffic light in HSV space
    # Measured green traffic light HSV: 56, 202, 214
    lower_green_light = np.array([51, 192, 70]) 
    upper_green_light = np.array([61, 212, 224])


    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV) 

    blue_line_mask = cv2.inRange(hsv, lower_blue_line, upper_blue_line)
    orange_line_mask = cv2.inRange(hsv, lower_orange_line, upper_orange_line)
    red_light_mask = cv2.inRange(hsv, lower_red_light, upper_red_light)
    green_light_mask = cv2.inRange(hsv, lower_green_light, upper_green_light)
        
    blue_line_y, blue_line_size = average_y_coordinate(blue_line_mask)
    orange_line_y, orange_line_size = average_y_coordinate(orange_line_mask)
    red_light_x, red_light_size = average_x_coordinate(red_light_mask)
    green_light_x, green_light_size = average_x_coordinate(green_light_mask)

    blue_line_info: tuple[cv2.typing.MatLike, int, int] = (blue_line_mask, blue_line_y, blue_line_size)
    orange_line_info: tuple[cv2.typing.MatLike, int, int] = (orange_line_mask, orange_line_y, orange_line_size)
    red_light_info: tuple[cv2.typing.MatLike, int, int] = (red_light_mask, red_light_x, red_light_size)
    green_light_info: tuple[cv2.typing.MatLike, int, int] = (green_light_mask, green_light_x, green_light_size)

    return blue_line_info, orange_line_info, red_light_info, green_light_info

def show_window(image: cv2.typing.MatLike,
                blue_line_info: tuple[cv2.typing.MatLike, int, int],
                orange_line_info: tuple[cv2.typing.MatLike, int, int],
                red_light_info: tuple[cv2.typing.MatLike, int, int],
                green_light_info: tuple[cv2.typing.MatLike, int, int] 
    ) -> bool:
    """
    Display the processed image with overlayed lines and masks, and check for user input to break the loop.

    Args:
        image (cv2.typing.MatLike): The input image to be processed.
        blue_line_info (tuple): A tuple containing the mask, y-coordinate, and size of the blue line.
        orange_line_info (tuple): A tuple containing the mask, y-coordinate, and size of the orange line.
        red_light_info (tuple): A tuple containing the mask, x-coordinate, and size of the red light.
        green_light_info (tuple): A tuple containing the mask, x-coordinate, and size of the green light.

    Returns:
        bool: True if the loop needs to be broken, i.e., if the 'q' key is pressed.
    """
    blue_line_mask, blue_line_y, blue_line_size = blue_line_info
    orange_line_mask, orange_line_y, orange_line_size = orange_line_info
    red_light_mask, red_light_x, red_light_size = red_light_info
    green_light_mask, green_light_x, green_light_size = green_light_info

    blue_line_result = cv2.bitwise_and(image, image, mask = blue_line_mask)
    orange_line_result = cv2.bitwise_and(image, image, mask = orange_line_mask)
    red_light_result = cv2.bitwise_and(image, image, mask = red_light_mask)
    green_light_result = cv2.bitwise_and(image, image, mask = green_light_mask)

    result = cv2.bitwise_or(blue_line_result, orange_line_result)
    result = cv2.bitwise_or(result, red_light_result)
    result = cv2.bitwise_or(result, green_light_result)

    # # Draw horizontal lines for blue and orange lines
    # if blue_line_y is not None:
    #     cv2.line(result, (0, int(blue_line_y)), (image.shape[1], int(blue_line_y)), (255, 0, 0), math.ceil(blue_line_size/100))
    # if orange_line_y is not None:
    #     cv2.line(result, (0, int(orange_line_y)), (image.shape[1], int(orange_line_y)), (0, 165, 255), math.ceil(orange_line_size/100))

    # # Draw vertical lines for red and green lights
    # if red_light_x is not None:
    #     cv2.line(result, (int(red_light_x), 0), (int(red_light_x), image.shape[0]), (0, 0, 255), math.ceil(red_light_size/100))
    # if green_light_x is not None: 
    #     cv2.line(result, (int(green_light_x), 0), (int(green_light_x), image.shape[0]), (0, 255, 0), math.ceil(green_light_size/100))

    # cv2.imshow('image', image)
    cv2.imshow('result', result)

    return cv2.waitKey(1) & 0xFF == ord('q')

init_gyro = None
def main():
    global communicator, init_gyro
    last_update_time = time.time()

    communicator.ser.reset_output_buffer()
    communicator.ser.reset_input_buffer()
    time.sleep(0.100)
    communicator.send_floats((0, 0))
    time.sleep(0.100)
    communicator.ser.reset_input_buffer()
    time.sleep(0.100)
    communicator.send_floats((0, 0))

    try:
        while True:
            received_floats = communicator.receive_floats(4)
            communicator.ser.reset_input_buffer()
            data = []
            if received_floats:
                for received_float in received_floats:
                    data.append(received_float)

                current_time = time.time()
                delta_time = current_time - last_update_time
                last_update_time = current_time

                image = picam2.capture_array()
                image = cv2.rotate(image, cv2.ROTATE_180)
                image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

                ultrasonic_info = (data[0], 0, data[1], data[2])

                gyro_info =  data[3]
                if init_gyro is None: init_gyro = gyro_info

                gyro_info = round(init_gyro - gyro_info) % 360

                blue_line_info, orange_line_info, red_light_info, green_light_info = process_image(image)
                result = process_data_open(ultrasonic_info, gyro_info, blue_line_info, orange_line_info, red_light_info, green_light_info, delta_time)
                # result = process_data_obstacle(ultrasonic_info, gyro_info, blue_line_info, orange_line_info, red_light_info, green_light_info, delta_time)

                speed_target = 0
                steering_percent = 0 
                if type(result) is bool:
                    if result == False:
                        communicator.send_floats((0, 0))
                        break
                else:
                    speed_target, steering_percent = result

                print(received_floats)
                communicator.send_floats((steering_percent, speed_target))
                communicator.ser.flush()
            else:
                print("Arduino Timeout!")
                # if show_window(image, blue_line_info, orange_line_info, red_light_info, green_light_info): break
    except Exception as e:
        print(e)
    finally:
        cv2.destroyAllWindows()
        communicator.send_floats((0, 0))
        communicator.ser.flush()
        time.sleep(0.1)
        communicator.close()
        time.sleep(0.5)
        print("Stopped!")

if __name__ == "__main__":
    main()
