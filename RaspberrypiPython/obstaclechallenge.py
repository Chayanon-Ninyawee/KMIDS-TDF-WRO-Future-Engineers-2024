import cv2
import math
import time
import pidcontroller
from config import *
from utils import *
from enum import Enum

# TODO: Add Scenario where pink wall is after the turning line

# Constants
MAX_HEADING_ERROR = 30.0
IDEAL_OUTER_WALL_DISTANCE = 0.5 - (LEFT_RIGHT_ULTRASONIC_DISTANCE / 2.0)
BLUE_ORANGE_SIZE_DIFF_THRESHOLD = 1000
ULTRASONIC_THRESHOLD = 0.70
ULTRASONIC_TURN_TIME_WINDOW = 0.3
TURN_COOLDOWN_TIME = 3

ULTRASONIC_TIGHT_THRESHOLD = 1.10
ULTRASONIC_TIGHT_TURN_TIME_WINDOW = 0.3
TIGHT_TURN_COOLDOWN_TIME = 2.7

LAPS_TO_STOP = 3

TRAFFIC_LIGHT_SIZE_THRESHOLD = 4000
TRAFFIC_LIGHT_Y_THRESHOLD = CAMERA_HEIGHT*0.8
TRAFFIC_LIGHT_COOLDOWN_TIME = 2.0
TIGHT_TURN_ULTRASONIC_THRESHOLD_1 = 0.90
TIGHT_TURN_ULTRASONIC_THRESHOLD_2 = 0.32
TIGHT_TURN_ULTRASONIC_THRESHOLD_NO = 0.60
TIGHT_TURN_LINGER_TIME = 2.0

TRAFFIC_LIGHT_HEADING_CORRECTION = 70
TRAFFIC_LIGHT_HEADING_ERROR_THRESHOLD = 10
RED_DISTANCE_FROM_RIGHT = 0.25
RED_WALL_DISTANCE_FROM_RIGHT = 0.18
GREEN_DISTANCE_FROM_LEFT = 0.25
GREEN_WALL_DISTANCE_FROM_LEFT = 0.18

UTURN_ULTRASONIC_THRESHOLD = 0.5
UTURN_HEADING_CORRECTION = 120
UTURN_HEADING_ERROR_THRESHOLD = 5

PINK_THRESHOLD = 12000
NEW_RED_DISTANCE_FROM_RIGHT = 0.30
NEW_RED_WALL_DISTANCE_FROM_RIGHT = 0.30
NEW_GREEN_DISTANCE_FROM_LEFT = 0.30
NEW_GREEN_WALL_DISTANCE_FROM_LEFT = 0.30

# PID Controllers
heading_pid = pidcontroller.PIDController(kp=0.1, ki=0.0, kd=0.01)
wall_distance_pid = pidcontroller.PIDController(kp=200.0, ki=0.0, kd=0)

class State(Enum):
    DO_NOTHING = -1
    NORMAL = 0
    TRAFFIC_TURNING = 1
    TRAFFIC_TURNING_BACK = 2
    TRAFFIC_TIGHT_TURNING = 3
    TRAFFIC_TIGHT_TURNING_LINGER = 4
    UTURNING = 5

# State variables
current_state = State.NORMAL
is_linger = False

suggested_heading = 0
is_clockwise = None
turn_amount = 0

ultrasonic_last_time_list = [0.0, 0.0, 0.0]

last_closest_block_color = None
is_last_closest_block_color_same = False
ideal_outer_wall_distance_override = IDEAL_OUTER_WALL_DISTANCE

ultrasonic_tight_last_time_list = [0.0, 0.0, 0.0]
is_tight_turn_ending = False
tight_turn_ending_last_time_list = [0.0, 0.0, 0.0]
last_tight_turn_closest_block_color = None

traffic_light_1_0_list = []
traffic_light_1_3_list = []
traffic_light_2_0_list = []

is_uturning = False
uturning_phase = 0

is_parking_here = False
is_parking_left = None

def process_data_obstacle(ultrasonic_info: tuple[int, int, int, int],
                      gyro_info: float,
                      image: cv2.typing.MatLike,
                      delta_time: float
                      ) -> tuple[float, float]:
    global current_state, is_linger
    global heading_pid, wall_distance_pid
    global suggested_heading, is_clockwise, turn_amount
    global ultrasonic_last_time_list
    global last_closest_block_color, is_last_closest_block_color_same, ideal_outer_wall_distance_override
    global is_tight_turn_ending, ultrasonic_tight_last_time_list, last_tight_turn_closest_block_color
    global traffic_light_1_0_list, traffic_light_1_3_list, traffic_light_2_0_list
    global is_uturning, uturning_phase
    global is_parking_here, is_parking_left

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info

    blue_line_y, blue_line_size, orange_line_y, orange_line_size, closest_block_x, closest_block_y, closest_block_lowest_y, closest_block_size, closest_block_color, pink_x, pink_y, pink_size = ImageProcessor.process_image(image)

    # cv2.line(image, (closest_block_x, 0), (closest_block_x, CAMERA_HEIGHT), (0, 0, 255), 3)
    # cv2.line(image, (0, closest_block_lowest_y), (CAMERA_WIDTH, closest_block_lowest_y), (0, 0, 255), 3)
    # cv2.line(image, (0, blue_line_y), (CAMERA_WIDTH, blue_line_y), (255, 0, 0), 3)
    # cv2.line(image, (0, orange_line_y), (CAMERA_WIDTH, orange_line_y), (255, 255, 0), 3)
    # cv2.imshow('image', image)
    # cv2.waitKey(1)

    heading_error = normalize_angle_error(suggested_heading - gyro_info)

    print(f'{is_clockwise} {turn_amount} {suggested_heading} {current_state} {traffic_light_1_0_list} {traffic_light_1_3_list} {traffic_light_2_0_list}')

    if is_clockwise is None:
        if blue_line_size is not None and orange_line_size is not None:
            if blue_line_size - orange_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = False
            elif orange_line_size - blue_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = True
    
    is_traffic_turning = False
    if closest_block_color is not None and closest_block_size >= TRAFFIC_LIGHT_SIZE_THRESHOLD:
        if blue_line_y is None and orange_line_y is None:
            is_traffic_turning = True
        elif blue_line_y is not None:
            is_traffic_turning = closest_block_lowest_y > blue_line_y
        elif orange_line_y is not None:
            is_traffic_turning = closest_block_lowest_y > orange_line_y
        elif blue_line_y is not None and orange_line_y is not None:
            is_traffic_turning = closest_block_lowest_y > orange_line_y and closest_block_lowest_y > blue_line_y

    if is_linger:
        pass
    elif execute_with_timing_conditions(
        is_tight_turn_ending,
        tight_turn_ending_last_time_list,
        linger_duration=TIGHT_TURN_LINGER_TIME
    ):
        current_state = State.TRAFFIC_TIGHT_TURNING_LINGER
    elif turn_amount >= 4*LAPS_TO_STOP:
        return False
    elif is_traffic_turning:
        current_state = State.TRAFFIC_TURNING
    elif execute_with_timing_conditions( # For UTURNING and TIGHT_TURNING
        front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_TIGHT_THRESHOLD,
        ultrasonic_tight_last_time_list,
        cooldown_duration=TIGHT_TURN_COOLDOWN_TIME,
        time_window=ULTRASONIC_TIGHT_TURN_TIME_WINDOW
    ):
        last_traffic_light_color = None

        if turn_amount == 7:
            if len(traffic_light_1_0_list) == 1 and len(traffic_light_2_0_list) == 1:
                last_traffic_light_color = traffic_light_1_3_list[-1]
            elif len(traffic_light_1_0_list) == 1 and len(traffic_light_2_0_list) == 2:
                last_traffic_light_color = traffic_light_2_0_list[0]
            else:
                print(f'{traffic_light_1_0_list} {traffic_light_1_3_list} {traffic_light_2_0_list}')

        if last_traffic_light_color == 'red':
            current_state = State.UTURNING
            ultrasonic_last_time_list[0] = time.time() # Reset ultrasonic_last_time_list cooldown
        elif (is_clockwise and last_closest_block_color == 'red') or (not is_clockwise and last_closest_block_color == 'green'):
            current_state = State.TRAFFIC_TIGHT_TURNING
            ultrasonic_last_time_list[0] = time.time() # Reset ultrasonic_last_time_list cooldown
        else:
            current_state = State.NORMAL
    else:
        current_state = State.NORMAL

    red_distance_from_right = RED_DISTANCE_FROM_RIGHT
    red_wall_distance_from_right = RED_WALL_DISTANCE_FROM_RIGHT
    green_distance_from_left = GREEN_DISTANCE_FROM_LEFT
    green_wall_distance_from_left = GREEN_WALL_DISTANCE_FROM_LEFT

    if pink_size is not None:
        if pink_size >= PINK_THRESHOLD:
            is_parking_here = True

            if current_state == State.NORMAL:
                current_state = State.DO_NOTHING
    
    if is_parking_here:
        if is_clockwise is None:
            if is_parking_left is not None:
                if is_parking_left:
                    green_distance_from_left = NEW_GREEN_DISTANCE_FROM_LEFT
                    green_wall_distance_from_left = NEW_GREEN_WALL_DISTANCE_FROM_LEFT
                    if ideal_outer_wall_distance_override == GREEN_WALL_DISTANCE_FROM_LEFT:
                        ideal_outer_wall_distance_override = NEW_GREEN_WALL_DISTANCE_FROM_LEFT
                else:
                    red_distance_from_right = NEW_RED_DISTANCE_FROM_RIGHT
                    red_wall_distance_from_right = NEW_RED_WALL_DISTANCE_FROM_RIGHT
                    if ideal_outer_wall_distance_override == RED_WALL_DISTANCE_FROM_RIGHT:
                        ideal_outer_wall_distance_override = NEW_RED_WALL_DISTANCE_FROM_RIGHT
            elif pink_x is not None:
                if pink_x >= CAMERA_WIDTH*0.5:
                    green_distance_from_left = NEW_GREEN_DISTANCE_FROM_LEFT
                    green_wall_distance_from_left = NEW_GREEN_WALL_DISTANCE_FROM_LEFT
                    is_parking_left = True
                    if ideal_outer_wall_distance_override == GREEN_WALL_DISTANCE_FROM_LEFT:
                        ideal_outer_wall_distance_override = NEW_GREEN_WALL_DISTANCE_FROM_LEFT
                else:
                    red_distance_from_right = NEW_RED_DISTANCE_FROM_RIGHT
                    red_wall_distance_from_right = NEW_RED_WALL_DISTANCE_FROM_RIGHT
                    is_parking_left = False
                    if ideal_outer_wall_distance_override == RED_WALL_DISTANCE_FROM_RIGHT:
                        ideal_outer_wall_distance_override = NEW_RED_WALL_DISTANCE_FROM_RIGHT
        else:
            if is_clockwise:
                green_distance_from_left = NEW_GREEN_DISTANCE_FROM_LEFT
                green_wall_distance_from_left = NEW_GREEN_WALL_DISTANCE_FROM_LEFT
                if ideal_outer_wall_distance_override == GREEN_WALL_DISTANCE_FROM_LEFT:
                    ideal_outer_wall_distance_override = NEW_GREEN_WALL_DISTANCE_FROM_LEFT
            else:
                red_distance_from_right = NEW_RED_DISTANCE_FROM_RIGHT
                red_wall_distance_from_right = NEW_RED_WALL_DISTANCE_FROM_RIGHT
                if ideal_outer_wall_distance_override == RED_WALL_DISTANCE_FROM_RIGHT:
                    ideal_outer_wall_distance_override = NEW_RED_WALL_DISTANCE_FROM_RIGHT

    speed = 1.00
    heading_correction_override = None

    if current_state == State.NORMAL:
        if is_clockwise is not None:
            if execute_with_timing_conditions(
                front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_THRESHOLD,
                ultrasonic_last_time_list,
                cooldown_duration=TURN_COOLDOWN_TIME,
                time_window=ULTRASONIC_TURN_TIME_WINDOW
            ):
                if is_clockwise:
                    suggested_heading += 90
                else:
                    suggested_heading -= 90
                suggested_heading %= 360
                last_closest_block_color = None
                is_last_closest_block_color_same = False
                ideal_outer_wall_distance_override = IDEAL_OUTER_WALL_DISTANCE
                is_parking_here = False
                turn_amount += 1
    elif current_state == State.TRAFFIC_TURNING:
        if not is_linger and not is_last_closest_block_color_same:
            is_last_closest_block_color_same = closest_block_color == last_closest_block_color

            last_closest_block_color = closest_block_color
            if turn_amount == 0:
                traffic_light_1_0_list.append(closest_block_color)
            elif turn_amount == 3:
                traffic_light_1_3_list.append(closest_block_color)
            elif turn_amount == 4:
                traffic_light_2_0_list.append(closest_block_color)

        is_linger = True

        if not is_last_closest_block_color_same:
            speed = 0.70
            traffic_light_heading_correction = None
            is_ultrasonic_reach = None

            if last_closest_block_color == 'red':
                traffic_light_heading_correction = TRAFFIC_LIGHT_HEADING_CORRECTION
                if is_clockwise is None or is_clockwise == True:
                    is_ultrasonic_reach = (back_ultrasonic + FRONT_BACK_ULTRASONIC_DISTANCE) * math.sin(math.radians(abs(heading_error))) >= 1.0 - red_distance_from_right
                else:
                    is_ultrasonic_reach = (front_ultrasonic) * math.sin(math.radians(abs(heading_error))) <= red_distance_from_right
            elif last_closest_block_color == 'green':
                traffic_light_heading_correction = -TRAFFIC_LIGHT_HEADING_CORRECTION
                if is_clockwise is None or is_clockwise == True:
                    is_ultrasonic_reach = (front_ultrasonic) * math.sin(math.radians(abs(heading_error))) <= green_distance_from_left
                else:
                    is_ultrasonic_reach = (back_ultrasonic + FRONT_BACK_ULTRASONIC_DISTANCE) * math.sin(math.radians(abs(heading_error))) >= 1.0 - green_distance_from_left

            if traffic_light_heading_correction == None or is_ultrasonic_reach == None:
                raise ValueError(f'traffic_light_heading_correction: {traffic_light_heading_correction}, is_ultrasonic_reach: {is_ultrasonic_reach}')

            if abs(heading_error + traffic_light_heading_correction) <= TRAFFIC_LIGHT_HEADING_ERROR_THRESHOLD and is_ultrasonic_reach:
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
        speed = 0.70
        heading_correction_override = 0

        if abs(heading_error) <= TRAFFIC_LIGHT_HEADING_ERROR_THRESHOLD:
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
    elif current_state == State.TRAFFIC_TIGHT_TURNING:
        speed = 0.70
        heading_correction_override = 0

        is_linger = True

        if last_tight_turn_closest_block_color is None:
            last_tight_turn_closest_block_color = closest_block_color

        is_ultrasonic_reach = None
        new_ideal_outer_wall_distance_override = None

        if last_tight_turn_closest_block_color == 'red':
            if is_clockwise:
                is_ultrasonic_reach = front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_1
                new_ideal_outer_wall_distance_override = 1.0 - red_wall_distance_from_right - LEFT_RIGHT_ULTRASONIC_DISTANCE
            else:
                is_ultrasonic_reach = front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_2
                new_ideal_outer_wall_distance_override = red_wall_distance_from_right
        elif last_tight_turn_closest_block_color == 'green':
            if is_clockwise:
                is_ultrasonic_reach = front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_2
                new_ideal_outer_wall_distance_override = green_wall_distance_from_left
            else:
                is_ultrasonic_reach = front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_1
                new_ideal_outer_wall_distance_override = 1.0 - green_wall_distance_from_left - LEFT_RIGHT_ULTRASONIC_DISTANCE
        else:
            is_ultrasonic_reach = front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_NO
            new_ideal_outer_wall_distance_override = IDEAL_OUTER_WALL_DISTANCE



        if is_ultrasonic_reach == None:
            raise ValueError(f'is_ultrasonic_reach: {is_ultrasonic_reach}, new_ideal_outer_wall_distance_override: {new_ideal_outer_wall_distance_override}')
        
        if is_ultrasonic_reach:
            if is_clockwise:
                suggested_heading += 90
            else:
                suggested_heading -= 90
            suggested_heading %= 360
            last_closest_block_color = last_tight_turn_closest_block_color
            is_last_closest_block_color_same = False
            ideal_outer_wall_distance_override = new_ideal_outer_wall_distance_override
            is_parking_here = False
            turn_amount += 1
            is_tight_turn_ending = True
            is_linger = False
    elif current_state == State.TRAFFIC_TIGHT_TURNING_LINGER:
        speed = 0.70

        if is_tight_turn_ending and last_tight_turn_closest_block_color is not None:
            if turn_amount == 0:
                traffic_light_1_0_list.append(last_tight_turn_closest_block_color)
            elif turn_amount == 3:
                traffic_light_1_3_list.append(last_tight_turn_closest_block_color)
            elif turn_amount == 4:
                traffic_light_2_0_list.append(last_tight_turn_closest_block_color)

        last_tight_turn_closest_block_color = None
        is_tight_turn_ending = False
    elif current_state == State.UTURNING:
        speed = 0.60
        is_linger = True

        if uturning_phase == 0:
            heading_correction_override = 0
            if front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= UTURN_ULTRASONIC_THRESHOLD:
                uturning_phase = 1

        if uturning_phase == 1:
            if last_closest_block_color == 'red':
                uturn_heading_correction_override = -UTURN_HEADING_CORRECTION
            elif last_closest_block_color == 'green':
                uturn_heading_correction_override = UTURN_HEADING_CORRECTION

            heading_correction_override = uturn_heading_correction_override

            if abs(heading_error + uturn_heading_correction_override) <= UTURN_HEADING_ERROR_THRESHOLD:
                uturning_phase = 2
        
        if uturning_phase == 2:
            speed = -0.60
            heading_correction_override = 180

            if abs(abs(heading_error) - 180) <= UTURN_HEADING_ERROR_THRESHOLD:
                uturning_phase = 0
                is_linger = False
                is_clockwise = not is_clockwise
                turn_amount = 8
                speed = 0.0
                suggested_heading = 270
                ideal_outer_wall_distance_override = IDEAL_OUTER_WALL_DISTANCE
                last_closest_block_color = None
                is_last_closest_block_color_same = False


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

    return speed, steering_percent


class ImageProcessor:
    @staticmethod
    def process_image(image):
        # Convert image to HSV color space
        hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
        
        # Create masks for blue and orange colors
        mask_blue = cv2.inRange(hsv_image, LOWER_BLUE_LINE, UPPER_BLUE_LINE)
        mask_orange = cv2.inRange(hsv_image, LOWER_ORANGE_LINE, UPPER_ORANGE_LINE)

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
        
        # Get centroids and areas for red and green blocks
        red_blocks = [ImageProcessor.get_centroid_and_area(c) for c in contours_red if ImageProcessor.get_centroid_and_area(c)[0] is not None]
        green_blocks = [ImageProcessor.get_centroid_and_area(c) for c in contours_green if ImageProcessor.get_centroid_and_area(c)[0] is not None]
        
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