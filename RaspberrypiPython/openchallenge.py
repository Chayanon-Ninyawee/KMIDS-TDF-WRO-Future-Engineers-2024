import cv2
import math
import time
import pidcontroller
from config import *
from utils import *

# Constants
MAX_HEADING_ERROR = 30.0
IDEAL_OUTER_WALL_DISTANCE = 0.38
BLUE_ORANGE_SIZE_DIFF_THRESHOLD = 1000
ULTRASONIC_THRESHOLD = 0.7
ULTRASONIC_TURN_TIME_WINDOW = 0.2
TURN_COOLDOWN_TIME = 4

LAPS_TO_STOP = 3
ULTRASONIC_STOP_THRESHOLD = 0.95 # (BTW CHANGE TO USE BACK SENSOR INSTEAD)
ULTRASONIC_STOP_TIME_WINDOW = 0.1
STOP_COOLDOWN_TIME = 3 # Must be less than TURN_COOLDOWN_TIME

# PID Controllers
heading_pid = pidcontroller.PIDController(kp=0.07, ki=0, kd=0)
wall_distance_pid = pidcontroller.PIDController(kp=150.0, ki=0, kd=0)

# State variables
last_left_ultrasonic = IDEAL_OUTER_WALL_DISTANCE
last_right_ultrasonic = IDEAL_OUTER_WALL_DISTANCE

suggested_heading = 0
is_clockwise = None
last_turn_time = 0
turn_amount = 0

ultrasonic_last_time_list = [0.0, 0.0, 0.0]

def process_data_open(ultrasonic_info: tuple[int, int, int, int],
                      gyro_info: float,
                      image: cv2.typing.MatLike,
                      delta_time: float
                      ) -> tuple[float, float]:
    """
    Process the sensor data to determine the power and steering percentages to keep the car driving parallel to the walls.

    Args:
        ultrasonic_info (tuple): A tuple containing the ultrasonic sensor readings for the front, back, left, and right directions.
        gyro_info (float): The gyroscope reading indicating the current heading.
        image (cv2.typing.MatLike): The captured image from the camera.
        delta_time (float): The time elapsed since the last update.

    Returns:
        tuple: A tuple containing the power percentage and steering percentage to keep the car driving parallel to the walls.
            - speed_target (float): The target speed of the car.
            - steering_percent (float): The steering percentage, ranging from -1.00 to 1.00, where 1.00 is full right and -1.00 is full left.
    """
    global last_left_ultrasonic, last_right_ultrasonic
    global heading_pid, wall_distance_pid
    global suggested_heading, is_clockwise, last_turn_time, turn_amount
    global ultrasonic_last_time_list

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_manager(ultrasonic_info)

    blue_line_properties, orange_line_properties = ImageProcessor.process_image(image)
    _, blue_line_size = blue_line_properties
    _, orange_line_size = orange_line_properties

    heading_error = normalize_angle_error(suggested_heading - gyro_info)

    print(f'{is_clockwise} {turn_amount} {suggested_heading} {ultrasonic_info}')

    if is_clockwise is None:
        if blue_line_size is not None and orange_line_size is not None:
            if blue_line_size - orange_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = False
            elif orange_line_size - blue_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = True
    
    if is_clockwise is not None:
        if turn_amount >= 4*LAPS_TO_STOP:
            if (not back_ultrasonic == -1) and (back_ultrasonic > ULTRASONIC_STOP_THRESHOLD) and (abs(heading_error) <= 5):
                return False

        if execute_with_timing_conditions(
            (not front_ultrasonic == -1) and (front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_THRESHOLD),
            ultrasonic_last_time_list,
            cooldown_duration=TURN_COOLDOWN_TIME,
            time_window=ULTRASONIC_TURN_TIME_WINDOW
        ):
            if is_clockwise:
                suggested_heading += 90
            else:
                suggested_heading -= 90
            suggested_heading %= 360
            last_turn_time = time.time()
            turn_amount += 1

    wall_error = 0
    if is_clockwise is None:
        wall_error = (right_ultrasonic - left_ultrasonic) * math.cos(math.radians(abs(heading_error))) / 2.0
    else:
        if is_clockwise:
            wall_error = -left_ultrasonic * math.cos(math.radians(abs(heading_error))) + IDEAL_OUTER_WALL_DISTANCE
        else:
            wall_error = right_ultrasonic * math.cos(math.radians(abs(heading_error))) - IDEAL_OUTER_WALL_DISTANCE

    heading_correction = wall_distance_pid.update(wall_error, delta_time)
    heading_correction = max(min(heading_correction, MAX_HEADING_ERROR), -MAX_HEADING_ERROR)

    heading_error_correction = normalize_angle_error(heading_error + heading_correction)
    steering_adjustment = heading_pid.update(heading_error_correction, delta_time)
    steering_percent = max(min(steering_adjustment, 1.00), -1.00)

    last_left_ultrasonic = left_ultrasonic
    last_right_ultrasonic = right_ultrasonic

    return 1.00, steering_percent


def ultrasonic_manager(ultrasonic_info):
    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info

    if front_ultrasonic > 100:
        front_ultrasonic = -1
    if back_ultrasonic > 100:
        back_ultrasonic = -1
    if left_ultrasonic > 100:
        left_ultrasonic = -1
    if right_ultrasonic > 100:
        right_ultrasonic = -1

    if left_ultrasonic == -1:
        left_ultrasonic = last_left_ultrasonic
    
    if right_ultrasonic == -1:
        right_ultrasonic = last_right_ultrasonic
    
    return front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic

class ImageProcessor:
    @staticmethod
    def process_image(image):
        # Convert image to HSV color space
        hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        
        # Create masks for blue and orange colors
        mask_blue = cv2.inRange(hsv_image, LOWER_BLUE_LINE, UPPER_BLUE_LINE)
        mask_orange = cv2.inRange(hsv_image, LOWER_ORANGE_LINE, UPPER_ORANGE_LINE)
        
        return ImageProcessor.get_line_properties(mask_blue), ImageProcessor.get_line_properties(mask_orange)
    
    @staticmethod
    def get_line_properties(mask):
        coordinates = np.column_stack(np.where(mask > 0))
        # if coordinates.size == 0:
        #     return None, 0
        # average_y = int(np.mean(coordinates[:, 0]))

        # return average_y, coordinates.size
        return None, coordinates.size