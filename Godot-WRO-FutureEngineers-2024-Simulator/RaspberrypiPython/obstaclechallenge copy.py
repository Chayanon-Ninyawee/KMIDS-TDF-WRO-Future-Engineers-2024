import cv2
import math
import time
import pidcontroller
from config import *
from utils import *
from enum import Enum

# TODO: Add Scenario where pink wall is after the turning line

# Constants
MAX_HEADING_ERROR = 20.0
IDEAL_OUTER_WALL_DISTANCE = 0.5 - (LEFT_RIGHT_ULTRASONIC_DISTANCE / 2.0)
BLUE_ORANGE_SIZE_DIFF_THRESHOLD = 3000

LAPS_TO_STOP = 3

ULTRASONIC_THRESHOLD = 0.82
ULTRASONIC_TURN_TIME_WINDOW = 0.3
TURN_COOLDOWN_TIME = 4

ULTRASONIC_TIGHT_THRESHOLD = 1.10
ULTRASONIC_TIGHT_TURN_TIME_WINDOW = 0.3
TIGHT_TURN_COOLDOWN_TIME = 2.7


TRAFFIC_LIGHT_SIZE_THRESHOLD = 3000

# PID Controllers
heading_pid = pidcontroller.PIDController(kp=0.07, ki=0, kd=0)
wall_distance_pid = pidcontroller.PIDController(kp=10.0, ki=0, kd=0)

class State(Enum):
    DO_NOTHING = -1
    NORMAL_TURNING = 0
    TRAFFIC_TURNING = 1
    TRAFFIC_TURNING_BACK = 2
    TRAFFIC_TIGHT_TURNING = 3
    TRAFFIC_TIGHT_TURNING_LINGER = 4
    AVOID_PINK_WALL = 5
    UTURNING = 6
    FIND_PARKING_LOT = 7

# State variables
last_left_ultrasonic = IDEAL_OUTER_WALL_DISTANCE
last_right_ultrasonic = IDEAL_OUTER_WALL_DISTANCE

current_state = State.NORMAL_TURNING
is_linger = False

turn_amount = 0

suggested_heading = 0.0
is_clockwise = None

traffic_light_list = []
last_closest_block_color = None
is_last_closest_block_color_same = False # Have it so that it will remember for the whole way in the straight section

is_parking_here = False

ultrasonic_last_time_list = [time.time() - 1.0, 0.0, 0.0]
ultrasonic_tight_last_time_list = [time.time() - 0.5, 0.0, 0.0]

def process_data_obstacle(ultrasonic_info: tuple[int, int, int, int],
                      gyro_info: float,
                      image: cv2.typing.MatLike,
                      delta_time: float
                      ) -> tuple[float, float]:
    global heading_pid, wall_distance_pid
    global suggested_heading, is_clockwise
    global ultrasonic_last_time_list, ultrasonic_tight_last_time_list

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_manager(ultrasonic_info)

    blue_line_y, blue_line_size, orange_line_y, orange_line_size, closest_block_x, closest_block_y, closest_block_lowest_y, closest_block_size, closest_block_color, pink_x, pink_y, pink_size = ImageProcessor.process_image(image)

    # cv2.line(image, (closest_block_x, 0), (closest_block_x, CAMERA_HEIGHT), (0, 0, 255), 3)
    # cv2.line(image, (0, closest_block_lowest_y), (CAMERA_WIDTH, closest_block_lowest_y), (0, 0, 255), 3)
    # cv2.line(image, (0, blue_line_y), (CAMERA_WIDTH, blue_line_y), (255, 0, 0), 3)
    # cv2.line(image, (0, orange_line_y), (CAMERA_WIDTH, orange_line_y), (255, 255, 0), 3)
    # cv2.imshow('image', image)
    # cv2.waitKey(1)

    heading_error = normalize_angle_error(suggested_heading - gyro_info)

    if is_clockwise is None:
        if blue_line_size is not None and orange_line_size is not None:
            if blue_line_size - orange_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = False
            elif orange_line_size - blue_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = True

    is_ultrasonic = execute_with_timing_conditions(
        (not front_ultrasonic == -1) and (front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_THRESHOLD),
        ultrasonic_last_time_list,
        cooldown_duration=TURN_COOLDOWN_TIME,
        time_window=ULTRASONIC_TURN_TIME_WINDOW
    )

    is_ultrasonic_tight = execute_with_timing_conditions(
        (not front_ultrasonic == -1) and (front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_TIGHT_THRESHOLD),
        ultrasonic_tight_last_time_list,
        cooldown_duration=TIGHT_TURN_COOLDOWN_TIME,
        time_window=ULTRASONIC_TIGHT_TURN_TIME_WINDOW
    )
    
    infos = front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic, blue_line_y, blue_line_size, orange_line_y, orange_line_size, closest_block_x, closest_block_y, closest_block_lowest_y, closest_block_size, closest_block_color, pink_x, pink_y, pink_size, heading_error, is_ultrasonic, is_ultrasonic_tight

    print_infos(infos)

    # Optional Logic Start
    find_current_state(infos)

    speed, heading_correction_override, ideal_outer_wall_distance_override = execute_current_state(infos)
    # Optional Logic End

    wall_error = 0
    if is_clockwise is None:
        wall_error = (right_ultrasonic - left_ultrasonic) * math.cos(math.radians(abs(heading_error))) / 2.0
    else:
        if is_clockwise:
            wall_error = -left_ultrasonic * math.cos(math.radians(abs(heading_error))) + ideal_outer_wall_distance_override
        else:
            wall_error = right_ultrasonic * math.cos(math.radians(abs(heading_error))) - ideal_outer_wall_distance_override

    heading_correction = wall_distance_pid.update(wall_error, delta_time)
    heading_correction = max(min(heading_correction, MAX_HEADING_ERROR), -MAX_HEADING_ERROR)

    if heading_correction_override is not None: heading_correction = heading_correction_override

    heading_error_correction = normalize_angle_error(heading_error + heading_correction)
    steering_adjustment = heading_pid.update(heading_error_correction, delta_time)
    steering_percent = max(min(steering_adjustment, 1.00), -1.00)

    if speed < 0.0: steering_percent = -steering_percent

    last_left_ultrasonic = left_ultrasonic
    last_right_ultrasonic = right_ultrasonic

    return speed, steering_percent


def find_current_state(infos):
    global current_state, is_linger
    global turn_amount
    global suggested_heading, is_clockwise
    global last_closest_block_color

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic, blue_line_y, blue_line_size, orange_line_y, orange_line_size, closest_block_x, closest_block_y, closest_block_lowest_y, closest_block_size, closest_block_color, pink_x, pink_y, pink_size, heading_error, is_ultrasonic, is_ultrasonic_tight = infos
    
    print(f'{heading_error} {is_clockwise} {turn_amount} {suggested_heading} {current_state}')

    if is_linger:
        return
    
    if turn_amount >= 4*LAPS_TO_STOP:
        current_state = State.FIND_PARKING_LOT
        return

    if closest_block_color is not None:
        if closest_block_size >= TRAFFIC_LIGHT_SIZE_THRESHOLD:
            if blue_line_y is None and orange_line_y is None:
                current_state = State.TRAFFIC_TURNING
                return
            elif blue_line_y is not None and orange_line_y is not None:
                if closest_block_lowest_y > orange_line_y and closest_block_lowest_y > blue_line_y:
                    current_state = State.TRAFFIC_TURNING
                    return
            elif blue_line_y is not None:
                if closest_block_lowest_y > blue_line_y:
                    current_state = State.TRAFFIC_TURNING
                    return
            elif orange_line_y is not None:
                if closest_block_lowest_y > orange_line_y:
                    current_state = State.TRAFFIC_TURNING
                    return
    
    if is_ultrasonic_tight:
        if turn_amount == 7:
            # TODO: Check for UTURN
            pass

        if (is_clockwise and last_closest_block_color == 'red') or (not is_clockwise and last_closest_block_color == 'green'):
            current_state = State.TRAFFIC_TIGHT_TURNING
            ultrasonic_last_time_list[0] = time.time() # Reset ultrasonic_last_time_list cooldown
            return
    
    if is_ultrasonic and is_clockwise is not None:
        current_state = State.NORMAL_TURNING
        return
    
    current_state = State.DO_NOTHING
            
    
def execute_current_state(infos):
    global current_state, is_linger
    global turn_amount
    global suggested_heading, is_clockwise
    global traffic_light_list, last_closest_block_color, is_last_closest_block_color_same
    global is_parking_here
    global ultrasonic_last_time_list, ultrasonic_tight_last_time_list

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic, blue_line_y, blue_line_size, orange_line_y, orange_line_size, closest_block_x, closest_block_y, closest_block_lowest_y, closest_block_size, closest_block_color, pink_x, pink_y, pink_size, heading_error, is_ultrasonic, is_ultrasonic_tight = infos

    speed = 1.00
    heading_correction_override = None
    ideal_outer_wall_distance_override = IDEAL_OUTER_WALL_DISTANCE

    if current_state == State.NORMAL_TURNING:
        if is_clockwise:
            suggested_heading += 89.4
        else:
            suggested_heading -= 89.4
        suggested_heading %= 360
        last_closest_block_color = None
        is_last_closest_block_color_same = False
        ideal_outer_wall_distance_override = IDEAL_OUTER_WALL_DISTANCE
        is_parking_here = False
        turn_amount += 1
    elif current_state == State.TRAFFIC_TURNING:
        # TODO: NOT TESTED YET
        if not is_linger and not is_last_closest_block_color_same:
            is_last_closest_block_color_same = closest_block_color == last_closest_block_color

            last_closest_block_color = closest_block_color
            if turn_amount <= 3:
                traffic_light_list.append(closest_block_color)

        is_linger = True

        if not is_last_closest_block_color_same:
            speed = 0.50
            traffic_light_heading_correction = None
            is_ultrasonic_reach = None

            if last_closest_block_color == 'red':
                traffic_light_heading_correction = TRAFFIC_LIGHT_HEADING_CORRECTION
                if is_clockwise == True:
                    is_ultrasonic_reach = (not back_ultrasonic == -1) and ((back_ultrasonic + FRONT_BACK_ULTRASONIC_DISTANCE) * math.sin(math.radians(abs(heading_error))) >= 1.0 - red_distance_from_right)
                elif is_clockwise is None or is_clockwise == False:
                    is_ultrasonic_reach = (not front_ultrasonic == -1) and (front_ultrasonic * math.sin(math.radians(abs(heading_error))) <= red_distance_from_right)
            elif last_closest_block_color == 'green':
                traffic_light_heading_correction = -TRAFFIC_LIGHT_HEADING_CORRECTION
                if is_clockwise is None or is_clockwise == True:
                    is_ultrasonic_reach = (not front_ultrasonic == -1) and (front_ultrasonic * math.sin(math.radians(abs(heading_error))) <= green_distance_from_left)
                else:
                    is_ultrasonic_reach = (not back_ultrasonic == -1) and ((back_ultrasonic + FRONT_BACK_ULTRASONIC_DISTANCE) * math.sin(math.radians(abs(heading_error))) >= 1.0 - green_distance_from_left)

            if traffic_light_heading_correction == None or is_ultrasonic_reach == None:
                raise ValueError(f'traffic_light_heading_correction: {traffic_light_heading_correction}, is_ultrasonic_reach: {is_ultrasonic_reach}')
            
            is_heading_ok = False
            if ideal_outer_wall_distance_override == IDEAL_OUTER_WALL_DISTANCE:
                is_heading_ok = abs(heading_error + traffic_light_heading_correction) <= TRAFFIC_LIGHT_HEADING_ERROR_THRESHOLD_IN_MID
            else:
                is_heading_ok = abs(heading_error + traffic_light_heading_correction) <= TRAFFIC_LIGHT_HEADING_ERROR_THRESHOLD_IN

            if is_heading_ok and is_ultrasonic_reach:
                heading_correction_override = 0
                current_state = State.TRAFFIC_TURNING_BACK # Don't turn is_linger to False since it will change the state
            else:
                heading_correction_override = traffic_light_heading_correction
        else:
            heading_correction_override = 0

            if last_closest_block_color == 'red':
                if is_clockwise is None or is_clockwise == True:
                    ideal_outer_wall_distance_override = 1.0 - red_wall_distance_from_right - LEFT_RIGHT_ULTRASONIC_DISTANCE
                else:
                    ideal_outer_wall_distance_override = red_wall_distance_from_right
            elif last_closest_block_color == 'green':
                if is_clockwise is None or is_clockwise == True:
                    ideal_outer_wall_distance_override = green_wall_distance_from_left
                else:
                    ideal_outer_wall_distance_override = 1.0 - green_wall_distance_from_left - LEFT_RIGHT_ULTRASONIC_DISTANCE
        
            is_linger = False
    elif current_state == State.TRAFFIC_TURNING_BACK:
        pass
    elif current_state == State.TRAFFIC_TIGHT_TURNING:
        pass
    elif current_state == State.TRAFFIC_TIGHT_TURNING_LINGER:
        pass
    elif current_state == State.AVOID_PINK_WALL:
        pass
    elif current_state == State.UTURNING:
        pass

    return speed, heading_correction_override, ideal_outer_wall_distance_override


def print_infos(infos):
    global current_state, is_linger
    global turn_amount
    global suggested_heading, is_clockwise
    global last_closest_block_color

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic, blue_line_y, blue_line_size, orange_line_y, orange_line_size, closest_block_x, closest_block_y, closest_block_lowest_y, closest_block_size, closest_block_color, pink_x, pink_y, pink_size, heading_error, is_ultrasonic, is_ultrasonic_tight = infos

    print(f'{heading_error} {is_clockwise} {turn_amount} {suggested_heading} {current_state}')


def ultrasonic_manager(ultrasonic_info):
    global last_left_ultrasonic, last_right_ultrasonic

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

        contours_blue, _ = cv2.findContours(mask_blue, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        contours_orange, _ = cv2.findContours(mask_orange, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        large_contours_blue = [c for c in contours_blue if cv2.contourArea(c) > MIN_BLUE_LINE_AREA]
        large_contours_orange = [c for c in contours_orange if cv2.contourArea(c) > MIN_ORANGE_LINE_AREA]

        mask_blue = np.zeros_like(mask_blue)
        mask_blue = cv2.drawContours(mask_blue, large_contours_blue, -1, 255, thickness=cv2.FILLED)

        mask_orange = np.zeros_like(mask_orange)
        mask_orange = cv2.drawContours(mask_orange, large_contours_orange, -1, 255, thickness=cv2.FILLED)

        blue_coordinates = ImageProcessor.get_coordinates(mask_blue)
        blue_line_size = blue_coordinates.size
        blue_line_y = ImageProcessor.get_average_y(blue_coordinates)

        orange_coordinates = ImageProcessor.get_coordinates(mask_orange)
        orange_line_size = orange_coordinates.size
        orange_line_y = ImageProcessor.get_average_y(orange_coordinates)


        # Create masks for red and green colors
        mask_red1 = cv2.inRange(hsv_image, LOWER_RED1_LIGHT, UPPER_RED1_LIGHT)
        mask_red2 = cv2.inRange(hsv_image, LOWER_RED2_LIGHT, UPPER_RED2_LIGHT)
        mask_red = mask_red1 | mask_red2
        mask_green = cv2.inRange(hsv_image, LOWER_GREEN_LIGHT, UPPER_GREEN_LIGHT)

        # Find contours for red and green blocks
        contours_red, _ = cv2.findContours(mask_red, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        contours_green, _ = cv2.findContours(mask_green, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        large_contours_red = [c for c in contours_red if cv2.contourArea(c) > MIN_RED_LINE_AREA]
        large_contours_green = [c for c in contours_green if cv2.contourArea(c) > MIN_GREEN_LINE_AREA]
        
        # Get centroids and areas for red and green blocks
        red_blocks = [ImageProcessor.get_centroid_and_area(c) for c in large_contours_red if ImageProcessor.get_centroid_and_area(c)[0] is not None]
        green_blocks = [ImageProcessor.get_centroid_and_area(c) for c in large_contours_green if ImageProcessor.get_centroid_and_area(c)[0] is not None]
        
        closest_red_block = ImageProcessor.find_closest_block(red_blocks)
        closest_green_block = ImageProcessor.find_closest_block(green_blocks)

        closest_block, closest_block_color = ImageProcessor.get_closest_block_to_camera(closest_red_block, closest_green_block)

        closest_block_x = closest_block_y = closest_block_size = closest_block_lowest_y = None
        if closest_block is not None:
            closest_block_x = closest_block[0][0]
            closest_block_y = closest_block[0][1]
            closest_block_size = closest_block[1]
            closest_block_lowest_y = closest_block[2][1]
        

        mask_pink = cv2.inRange(hsv_image, LOWER_PINK_LIGHT, UPPER_PINK_LIGHT)

        pink_coordinates = ImageProcessor.get_coordinates(mask_pink)
        pink_size = pink_coordinates.size
        pink_x = ImageProcessor.get_average_x(pink_coordinates)
        pink_y = ImageProcessor.get_average_y(pink_coordinates)
        
        return blue_line_y, blue_line_size, orange_line_y, orange_line_size, closest_block_x, closest_block_y, closest_block_lowest_y, closest_block_size, closest_block_color, pink_x, pink_y, pink_size

    @staticmethod
    def get_coordinates(mask):
        return np.column_stack(np.where(mask > 0))
    
    @staticmethod
    def get_average_x(coordinates):
        if coordinates.size == 0:
            return None
        return int(np.mean(coordinates[:, 1]))
    
    @staticmethod
    def get_average_y(coordinates):
        if coordinates.size == 0:
            return None
        return int(np.mean(coordinates[:, 0]))

    @staticmethod
    def get_centroid_and_area(contour):
        M = cv2.moments(contour, True)
        if M["m00"] == 0:
            return None, 0
        cx = int(M["m10"] / M["m00"])
        cy = int(M["m01"] / M["m00"])
        area = cv2.contourArea(contour)
        return (cx, cy), area, tuple(contour[contour[:, :, 1].argmax()][0])

    @staticmethod
    def find_closest_block(blocks):
        if not blocks:
            return None
        
        # Sort blocks by area (descending) and then by y-coordinate of the centroid (ascending)
        sorted_blocks = sorted(blocks, key=lambda b: (-b[1], b[0][1]))
        
        return sorted_blocks[0]
    
    @staticmethod
    def get_closest_block_to_camera(red_block, green_block):
        if red_block is None and green_block is None:
            return None, None
        elif red_block is None:
            return green_block, 'green'
        elif green_block is None:
            return red_block, 'red'
        
        # Compare areas and y-coordinates to find the closest block
        if red_block[0][1] < green_block[0][1]:
            return green_block, 'green'
        elif red_block[0][1] > green_block[0][1]:
            return red_block, 'red'
        else:
            if red_block[1] > green_block[1]:
                return green_block, 'green'
            else:
                return red_block, 'red'