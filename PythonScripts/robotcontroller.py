import numpy as np
import cv2
import math
import time

import socketclient

from openchallenge import process_data_open
from obstaclechallenge import process_data_obstacle

from config import *
from utils import *

robot_data = socketclient.SocketClient('127.0.0.1', 12345, CAMERA_WIDTH, CAMERA_HEIGHT, 4*4, 4)


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
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV) 

    blue_line_mask = cv2.inRange(hsv_image, LOWER_BLUE_LINE, UPPER_BLUE_LINE)
    orange_line_mask = cv2.inRange(hsv_image, LOWER_ORANGE_LINE, UPPER_ORANGE_LINE)
    red_light_mask = cv2.inRange(hsv_image, LOWER_RED1_LIGHT, UPPER_RED1_LIGHT)
    green_light_mask = cv2.inRange(hsv_image, LOWER_GREEN_LIGHT, UPPER_GREEN_LIGHT)
        
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

            # closest_block = test_process_image(image)
            # display_closest_block(image, closest_block)

            # blue_line_info, orange_line_info, red_light_info, green_light_info = process_image(image)
            # result = process_data_open(ultrasonic_info, gyro_info, image, delta_time)
            result = process_data_obstacle(ultrasonic_info, gyro_info, image, delta_time)
            # result = (0, 0)

            speed_target = 0
            steering_percent = 0 
            if type(result) is bool:
                if result == False:
                    robot_data.send_data(0, 0)
                    break
            else:
                speed_target, steering_percent = result

            robot_data.send_data(speed_target*0.33, steering_percent)

            # cv2.imshow('image', image)
            # if cv2.waitKey(1) & 0xFF == ord('q'): break
    except Exception as e: print(e)
    finally:
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()