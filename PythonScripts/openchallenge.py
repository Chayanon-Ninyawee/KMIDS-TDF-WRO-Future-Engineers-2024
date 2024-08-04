import cv2
import math
import time
import pidcontroller
from config import *
from utils import *

# Constants
MAX_HEADING_ERROR = 30.0
IDEAL_WALL_DISTANCE = 0.38
BLUE_ORANGE_SIZE_DIFF_THRESHOLD = 1000
ULTRASONIC_THRESHOLD = 0.6
TURN_COOLDOWN_TIME = 3
LAPS_TO_STOP = 3
ULTRASONIC_STOP_THRESHOLD = 1.4 # Must be more than ULTRASONIC_THRESHOLD
STOP_COOLDOWN_TIME = 2 # Must be less than TURN_COOLDOWN_TIME

# PID Controllers
heading_pid = pidcontroller.PIDController(kp=0.1, ki=0.0, kd=0.01)
wall_distance_pid = pidcontroller.PIDController(kp=150.0, ki=0.0, kd=0)

# State variables
suggested_heading = 0
is_clockwise = None
last_turn_time = 0
turn_amount = 0

def process_data_open(ultrasonic_info: tuple[int, int, int, int],
                      gyro_info: float,
                      blue_line_info: tuple[cv2.typing.MatLike, int, int],
                      orange_line_info: tuple[cv2.typing.MatLike, int, int],
                      red_light_info: tuple[cv2.typing.MatLike, int, int],
                      green_light_info: tuple[cv2.typing.MatLike, int, int],
                      delta_time: float
                      ) -> tuple[float, float]:
    """
    Process the sensor data to determine the power and steering percentages to keep the car driving parallel to the walls.

    Args:
        ultrasonic_info (tuple): A tuple containing the ultrasonic sensor readings for the front, back, left, and right directions.
        gyro_info (float): The gyroscope reading indicating the current heading.
        blue_line_info (tuple): A tuple containing the mask, y-coordinate, and size of the blue line.
        orange_line_info (tuple): A tuple containing the mask, y-coordinate, and size of the orange line.
        red_light_info (tuple): A tuple containing the mask, x-coordinate, and size of the red light.
        green_light_info (tuple): A tuple containing the mask, x-coordinate, and size of the green light.
        delta_time (float): The time elapsed since the last update.

    Returns:
        tuple: A tuple containing the power percentage and steering percentage to keep the car driving parallel to the walls.
            - speed_target (float): The target speed of the car.
            - steering_percent (float): The steering percentage, ranging from -1.00 to 1.00, where 1.00 is full right and -1.00 is full left.
    """
    global heading_pid, wall_distance_pid
    global suggested_heading, is_clockwise, last_turn_time, turn_amount

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info
    blue_line_mask, blue_line_y, blue_line_size = blue_line_info
    orange_line_mask, orange_line_y, orange_line_size = orange_line_info

    # Calculate and normalize heading error
    heading_error = normalize_angle_error(gyro_info - suggested_heading)

    # Determine the direction of rotation
    if is_clockwise is None:
        if blue_line_size is not None and orange_line_size is not None:
            if blue_line_size - orange_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = False
            elif orange_line_size - blue_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = True
    else:
        if turn_amount >= 4*LAPS_TO_STOP:
            if front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_STOP_THRESHOLD and time.time() - last_turn_time > STOP_COOLDOWN_TIME:
                return False
        if front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_THRESHOLD and time.time() - last_turn_time > TURN_COOLDOWN_TIME:
            if is_clockwise:
                suggested_heading -= 90
            else:
                suggested_heading += 90
            suggested_heading %= 360
            last_turn_time = time.time()
            turn_amount += 1

    # Calculate wall error
    wall_error = 0
    if is_clockwise is None:
        wall_error = (right_ultrasonic - left_ultrasonic) * math.cos(math.radians(abs(heading_error))) / 2.0
    else:
        if is_clockwise:
            wall_error = -left_ultrasonic * math.cos(math.radians(abs(heading_error))) + IDEAL_WALL_DISTANCE
        else:
            wall_error = right_ultrasonic * math.cos(math.radians(abs(heading_error))) - IDEAL_WALL_DISTANCE

    # Apply wall distance PID controller
    heading_correction = wall_distance_pid.update(wall_error, delta_time)
    heading_correction = max(min(heading_correction, MAX_HEADING_ERROR), -MAX_HEADING_ERROR)

    # Adjust heading error with wall correction and normalize
    heading_error = normalize_angle_error(heading_error + heading_correction)

    # Apply heading PID controller
    steering_adjustment = heading_pid.update(heading_error, delta_time)
    steering_percent = max(min(steering_adjustment, 1.00), -1.00)

    return 0.33, steering_percent
