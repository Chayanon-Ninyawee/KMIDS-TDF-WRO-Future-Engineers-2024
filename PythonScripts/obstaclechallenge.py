import cv2
import math
import time
import pidcontroller
from config import *
from utils import *

# Constants
MAX_HEADING_ERROR = 30.0
IDEAL_WALL_DISTANCE = 0.51
BLUE_ORANGE_SIZE_DIFF_THRESHOLD = 1000
ULTRASONIC_THRESHOLD = 0.85
ULTRASONIC_TIME_THRESHOLD = 0.4
ULTRASONIC_ANGLE_THRESHOLD = 8
WALL_ERROR_ULTRASONIC_ANGLE_OVERRIDE_THRESHOLD = 0.5
TURN_COOLDOWN_TIME = 3
LAPS_TO_STOP = 3
ULTRASONIC_STOP_THRESHOLD = 1.4 # Must be more than ULTRASONIC_THRESHOLD
ULTRASONIC_STOP_TIME_THRESHOLD = 0.3
ULTRASONIC_STOP_ANGLE_THRESHOLD = 8
STOP_COOLDOWN_TIME = 2 # Must be less than TURN_COOLDOWN_TIME

TRAFFIC_LIGHT_THRESHOLD = 2000
TRAFFIC_LIGHT_DIFF_THRESHOLD = 1000
TRAFFIC_LIGHT_WALL_ERROR_BIAS = 0.30
TRAFFIC_LINGER_TIME = 1.0

# PID Controllers
heading_pid = pidcontroller.PIDController(kp=0.1, ki=0.0, kd=0.01)
wall_distance_pid = pidcontroller.PIDController(kp=150.0, ki=0.01, kd=0)

# State variables
suggested_heading = 0
is_clockwise = None
is_ultrasonic_below_threshold = False
ultrasonic_below_threshold_time = 0
is_ultrasonic_below_stop_threshold = False
ultrasonic_below_stop_threshold_time = 0
last_turn_time = 0
turn_amount = 0

last_red_time = 0
last_green_time = 0
last_wall_error_bias = 0

def process_data_obstacle(ultrasonic_info: tuple[int, int, int, int],
                      gyro_info: float,
                      blue_line_info: tuple[cv2.typing.MatLike, int, int],
                      orange_line_info: tuple[cv2.typing.MatLike, int, int],
                      red_light_info: tuple[cv2.typing.MatLike, int, int],
                      green_light_info: tuple[cv2.typing.MatLike, int, int],
                      delta_time: float
                      ) -> tuple[float, float]:
    global heading_pid, wall_distance_pid
    global suggested_heading, is_ultrasonic_below_threshold, ultrasonic_below_threshold_time, is_ultrasonic_below_stop_threshold, ultrasonic_below_stop_threshold_time, is_clockwise, last_turn_time, turn_amount
    global last_red_time, last_green_time, last_wall_error_bias

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info
    blue_line_mask, blue_line_y, blue_line_size = blue_line_info
    orange_line_mask, orange_line_y, orange_line_size = orange_line_info
    red_light_mask, red_light_x, red_light_size = red_light_info
    green_light_mask, green_light_x, green_light_size = green_light_info

    is_blue_exist = not blue_line_y == None and not blue_line_size == None
    is_orange_exist = not orange_line_y == None and not orange_line_size == None

    is_red_exist = not red_light_x == None and not red_light_size == None
    is_green_exist = not green_light_x == None and not green_light_size == None

    # Calculate and normalize heading error
    heading_error = normalize_angle_error(gyro_info - suggested_heading)

    if not front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_THRESHOLD:
        ultrasonic_below_threshold_time = time.time()
        is_ultrasonic_below_threshold = False
    elif time.time() - ultrasonic_below_threshold_time >= ULTRASONIC_TIME_THRESHOLD:
        if is_clockwise is None:
            is_ultrasonic_below_threshold = False
        else:
            if is_clockwise and (heading_error <= ULTRASONIC_ANGLE_THRESHOLD or left_ultrasonic * math.cos(math.radians(abs(heading_error))) >= WALL_ERROR_ULTRASONIC_ANGLE_OVERRIDE_THRESHOLD):
                is_ultrasonic_below_threshold = True
            elif not is_clockwise and (heading_error >= -ULTRASONIC_ANGLE_THRESHOLD or right_ultrasonic * math.cos(math.radians(abs(heading_error))) >= WALL_ERROR_ULTRASONIC_ANGLE_OVERRIDE_THRESHOLD):
                is_ultrasonic_below_threshold = True
            else:
                is_ultrasonic_below_threshold = False

    if front_ultrasonic * math.cos(math.radians(abs(heading_error))) > ULTRASONIC_STOP_THRESHOLD:
        ultrasonic_below_stop_threshold_time = time.time()
        is_ultrasonic_below_stop_threshold = False
    elif time.time() - ultrasonic_below_stop_threshold_time >= ULTRASONIC_STOP_TIME_THRESHOLD:
        if is_clockwise is None:
            is_ultrasonic_below_stop_threshold = False
        else:
            if is_clockwise and heading_error <= ULTRASONIC_STOP_ANGLE_THRESHOLD:
                is_ultrasonic_below_stop_threshold = True
            elif not is_clockwise and heading_error >= -ULTRASONIC_STOP_ANGLE_THRESHOLD:
                is_ultrasonic_below_stop_threshold = True
            else:
                is_ultrasonic_below_stop_threshold = False

    

    # Determine the direction of rotation
    if is_clockwise is None:
        if blue_line_size is not None and orange_line_size is not None:
            if blue_line_size - orange_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = False
            elif orange_line_size - blue_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = True
    else:
        if turn_amount >= 4*LAPS_TO_STOP:
            if is_ultrasonic_below_stop_threshold and time.time() - last_turn_time > STOP_COOLDOWN_TIME:
                return False
        if is_ultrasonic_below_threshold and time.time() - last_turn_time > TURN_COOLDOWN_TIME:
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

    is_red_behind_turn = False
    if is_red_exist and is_blue_exist and is_orange_exist:
        is_red_behind_turn = lowest_coordinate(red_light_mask) <= blue_line_y or lowest_coordinate(red_light_mask) <= orange_line_y
    
    is_green_behind_turn = False
    if is_green_exist and is_blue_exist and is_orange_exist:
        is_green_behind_turn = lowest_coordinate(green_light_mask) <= blue_line_y or lowest_coordinate(green_light_mask) <= orange_line_y

    wall_error_bias = 0
    # Add wall error bias depend on traffic light
    if is_red_exist and is_green_exist:
        if red_light_size >= TRAFFIC_LIGHT_THRESHOLD and red_light_size - green_light_size >= TRAFFIC_LIGHT_DIFF_THRESHOLD and not is_red_behind_turn:
            last_red_time = time.time()
            wall_error_bias = TRAFFIC_LIGHT_WALL_ERROR_BIAS
        elif time.time() - last_red_time <= TRAFFIC_LINGER_TIME:
            wall_error_bias = TRAFFIC_LIGHT_WALL_ERROR_BIAS
        elif green_light_size >= TRAFFIC_LIGHT_THRESHOLD and green_light_size - red_light_size >= TRAFFIC_LIGHT_DIFF_THRESHOLD and not is_green_behind_turn:
            last_green_time = time.time()
            wall_error_bias = -TRAFFIC_LIGHT_WALL_ERROR_BIAS
        elif time.time() - last_green_time <= TRAFFIC_LINGER_TIME:
            wall_error_bias = -TRAFFIC_LIGHT_WALL_ERROR_BIAS
        else:
            print("Not Implement This Yet!")
    elif is_red_exist:
        if red_light_size >= TRAFFIC_LIGHT_THRESHOLD and not is_red_behind_turn:
            last_red_time = time.time()
            wall_error_bias = TRAFFIC_LIGHT_WALL_ERROR_BIAS
    elif time.time() - last_red_time <= TRAFFIC_LINGER_TIME:
        wall_error_bias = TRAFFIC_LIGHT_WALL_ERROR_BIAS
    elif is_green_exist:
        if green_light_size >= TRAFFIC_LIGHT_THRESHOLD and not is_green_behind_turn:
            last_green_time = time.time()
            wall_error_bias = -TRAFFIC_LIGHT_WALL_ERROR_BIAS
    elif time.time() - last_green_time <= TRAFFIC_LINGER_TIME:
        wall_error_bias = -TRAFFIC_LIGHT_WALL_ERROR_BIAS

    last_wall_error_bias = wall_error_bias
    wall_error += wall_error_bias

    # Apply wall distance PID controller
    heading_correction = wall_distance_pid.update(wall_error, delta_time)
    heading_correction = max(min(heading_correction, MAX_HEADING_ERROR), -MAX_HEADING_ERROR)

    # Adjust heading error with wall correction and normalize
    heading_error = normalize_angle_error(heading_error + heading_correction)

    # Apply heading PID controller
    steering_adjustment = heading_pid.update(heading_error, delta_time)
    steering_percent = max(min(steering_adjustment, 1.00), -1.00)

    return 0.33, steering_percent
