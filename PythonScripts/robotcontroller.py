import numpy as np
import cv2
import math
import time

import socketclient

from openchallenge import process_data_open
from obstaclechallenge import process_data_obstacle

from config import *
from utils import *

robot_data = socketclient.SocketClient('127.0.0.1', 12345, camera_width, camera_height, 4*4, 4)


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
    # Measured blue line HSV: 111, 198, 165
    lower_blue_line = np.array([106, 188, 155]) 
    upper_blue_line = np.array([116, 208, 175])

    # Threshold of orange line in HSV space
    # Measured orange line HSV: 12, 232, 244
    lower_orange_line = np.array([7, 222, 234]) 
    upper_orange_line = np.array([17, 242, 254])

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

    # Draw horizontal lines for blue and orange lines
    if blue_line_y is not None:
        cv2.line(result, (0, int(blue_line_y)), (image.shape[1], int(blue_line_y)), (255, 0, 0), math.ceil(blue_line_size/100))
    if orange_line_y is not None:
        cv2.line(result, (0, int(orange_line_y)), (image.shape[1], int(orange_line_y)), (0, 165, 255), math.ceil(orange_line_size/100))

    # Draw vertical lines for red and green lights
    if red_light_x is not None:
        cv2.line(result, (int(red_light_x), 0), (int(red_light_x), image.shape[0]), (0, 0, 255), math.ceil(red_light_size/100))
    if green_light_x is not None: 
        cv2.line(result, (int(green_light_x), 0), (int(green_light_x), image.shape[0]), (0, 255, 0), math.ceil(green_light_size/100))

    # cv2.imshow('image', image)
    cv2.imshow('result', result)

    return cv2.waitKey(1) & 0xFF == ord('q')


def main():
    global robot_data
    last_update_time = time.time()

    try:
        while True:
            current_time = time.time()
            delta_time = current_time - last_update_time
            last_update_time = current_time

            robot_data.process_data()

            image = robot_data.get_image()
            ultrasonic_info = robot_data.get_ultasonic_data()
            gyro_info = robot_data.get_gyro_data()

            blue_line_info, orange_line_info, red_light_info, green_light_info = process_image(image)
            # result = process_data_open(ultrasonic_info, gyro_info, blue_line_info, orange_line_info, red_light_info, green_light_info, delta_time)
            result = process_data_obstacle(ultrasonic_info, gyro_info, blue_line_info, orange_line_info, red_light_info, green_light_info, delta_time)

            speed_target = 0
            steering_percent = 0 
            if type(result) is bool:
                if result == False:
                    robot_data.send_data(0, 0)
                    break
            else:
                speed_target, steering_percent = result

            robot_data.send_data(speed_target, steering_percent)

            # if show_window(image, blue_line_info, orange_line_info, red_light_info, green_light_info): break
    except Exception as e: print(e)
    finally:
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()