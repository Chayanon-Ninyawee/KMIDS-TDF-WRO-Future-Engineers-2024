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

ULTRASONIC_THRESHOLD = 0.73
ULTRASONIC_TIME_THRESHOLD = 0.4
ULTRASONIC_ANGLE_THRESHOLD = 8
TURN_COOLDOWN_TIME = 3

ULTRASONIC_THRESHOLD_2 = 1.20
TURN_COOLDOWN_TIME_2 = 2.7

LAPS_TO_STOP = 3
ULTRASONIC_STOP_THRESHOLD = 1.4 # Must be more than ULTRASONIC_THRESHOLD
ULTRASONIC_STOP_TIME_THRESHOLD = 0.3
ULTRASONIC_STOP_ANGLE_THRESHOLD = 8
STOP_COOLDOWN_TIME = 2 # Must be less than TURN_COOLDOWN_TIME

TRAFFIC_LIGHT_THRESHOLD = 3500
TRAFFIC_HEADING_CORRECTION = 70
TRAFFIC_HEADING_ERROR_THRESHOLD = 5
RED_DISTANCE_FROM_RIGHT = 0.35
RED_WALL_DISTANCE_FROM_RIGHT = 0.25
GREEN_DISTANCE_FROM_LEFT = 0.35
GREEN_WALL_DISTANCE_FROM_LEFT = 0.25

TIGHT_TURN_ULTRASONIC_THRESHOLD_1 = 1.15
TIGHT_TURN_ULTRASONIC_THRESHOLD_2 = 0.35
TIGHT_TURN_LINGER_TIME = 1.7

TRAFFIC_LIGHT_LIST_COOLDOWN_TIME = 3

# PID Controllers
heading_pid = pidcontroller.PIDController(kp=0.1, ki=0.0, kd=0.01)
wall_distance_pid = pidcontroller.PIDController(kp=150.0, ki=0.01, kd=0)

# State variables
suggested_heading = 0
is_clockwise = None
is_ultrasonic_below_threshold = False
ultrasonic_below_threshold_time = 0
ultrasonic_below_threshold_time_2 = 0
is_ultrasonic_below_stop_threshold = False
is_ultrasonic_below_threshold_2 = False
ultrasonic_below_stop_threshold_time = 0
last_turn_time = 0
turn_amount = 0

is_traffic_adjusting = False
is_traffic_adjusting_back = False
last_is_closest_block_red = None
ideal_wall_distance_override = IDEAL_WALL_DISTANCE
is_last_is_closest_block_red_same = False

is_tight_turn = False
is_tight_turn_counting_down = False
is_tight_turn_time = 0

traffic_light_area_1_list = []
traffic_light_area_1_3_list = []
traffic_light_area_2_list = []
last_traffic_light_time = 0
is_last_traffic_red = None

# TODO: Fix last traffic light
# TODO: Fix the heading that is change from clockwise negative to clockwise positive
def process_data_obstacle(ultrasonic_info: tuple[int, int, int, int],
                      gyro_info: float,
                      image: cv2.typing.MatLike,
                      delta_time: float
                      ) -> tuple[float, float]:
    global heading_pid, wall_distance_pid
    global suggested_heading, is_ultrasonic_below_threshold, ultrasonic_below_threshold_time, is_ultrasonic_below_stop_threshold, ultrasonic_below_stop_threshold_time, is_clockwise, last_turn_time, turn_amount
    global ultrasonic_below_threshold_time_2, is_ultrasonic_below_threshold_2
    global is_traffic_adjusting, is_traffic_adjusting_back, last_is_closest_block_red, ideal_wall_distance_override, is_last_is_closest_block_red_same
    global is_tight_turn, is_tight_turn_counting_down, is_tight_turn_time
    global traffic_light_area_1_list, traffic_light_area_2_list, last_traffic_light_time, is_last_traffic_red

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info
    blue_line_size, orange_line_size, closest_block_x, is_closest_block_red = process_image(image)

    # cv2.line(image, (closest_block_x, 0), (closest_block_x, CAMERA_HEIGHT), (0, 0, 255), 3)
    # cv2.imshow('image', image)
    # cv2.waitKey(1)

    print(f'1: {traffic_light_area_1_list} 2: {traffic_light_area_2_list}')

    # Calculate and normalize heading error
    heading_error = normalize_angle_error(gyro_info - suggested_heading)

    if is_clockwise is None:
        if blue_line_size is not None and orange_line_size is not None:
            if blue_line_size - orange_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = False
            elif orange_line_size - blue_line_size > BLUE_ORANGE_SIZE_DIFF_THRESHOLD:
                is_clockwise = True

    if turn_amount == 8 and is_last_traffic_red == None:
        if len(traffic_light_area_1_list) == 1 and len(traffic_light_area_2_list) == 1:
            if traffic_light_area_1_list[0] == traffic_light_area_2_list[0]:
                is_last_traffic_red = traffic_light_area_2_list[0]
            else:
                print("Error can't find last traffic")
        elif len(traffic_light_area_1_list) == 1 and len(traffic_light_area_2_list) == 2:
            is_last_traffic_red = traffic_light_area_2_list[0]
        
        print(f'1: {traffic_light_area_1_list} 2: {traffic_light_area_2_list}')

    heading_correction = 0
    speed = 1.00
    if is_tight_turn:
        speed = 0.40
        if is_tight_turn_counting_down:
            speed = 0.50
            if time.time() - is_tight_turn_time >= TIGHT_TURN_LINGER_TIME:
                is_tight_turn_counting_down = False
                is_tight_turn = False
        else:
            if closest_block_x is not None:
                if is_clockwise:
                    if is_closest_block_red:
                        if front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_1:
                            suggested_heading -= 90
                            suggested_heading %= 360
                            last_turn_time = time.time()
                            last_is_closest_block_red = True
                            ideal_wall_distance_override = (1.0 - RED_WALL_DISTANCE_FROM_RIGHT) + 0.05
                            is_last_is_closest_block_red_same = False
                            turn_amount += 1
                            is_tight_turn_time = time.time()
                            is_tight_turn_counting_down = True
                            if turn_amount == 4 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                                traffic_light_area_2_list.append(is_closest_block_red)
                                last_traffic_light_time = time.time()
                    else:
                        if front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_2:
                            suggested_heading -= 90
                            suggested_heading %= 360
                            last_turn_time = time.time()
                            last_is_closest_block_red = False
                            ideal_wall_distance_override = GREEN_WALL_DISTANCE_FROM_LEFT
                            is_last_is_closest_block_red_same = False
                            turn_amount += 1
                            is_tight_turn_time = time.time()
                            is_tight_turn_counting_down = True
                            if turn_amount == 4 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                                traffic_light_area_2_list.append(is_closest_block_red)
                                last_traffic_light_time = time.time()
                else:
                    if is_closest_block_red:
                        if front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_2:
                            suggested_heading += 90
                            suggested_heading %= 360
                            last_turn_time = time.time()
                            last_is_closest_block_red = True
                            ideal_wall_distance_override = RED_WALL_DISTANCE_FROM_RIGHT
                            is_last_is_closest_block_red_same = False
                            turn_amount += 1
                            is_tight_turn_time = time.time()
                            is_tight_turn_counting_down = True
                            if turn_amount == 4 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                                traffic_light_area_2_list.append(is_closest_block_red)
                                last_traffic_light_time = time.time()
                    else:
                        if front_ultrasonic * math.cos(math.radians(abs(heading_error))) <= TIGHT_TURN_ULTRASONIC_THRESHOLD_1:
                            suggested_heading += 90
                            suggested_heading %= 360
                            last_turn_time = time.time()
                            last_is_closest_block_red = False
                            ideal_wall_distance_override = (1.0 - GREEN_WALL_DISTANCE_FROM_LEFT) + 0.05
                            is_last_is_closest_block_red_same = False
                            turn_amount += 1
                            is_tight_turn_time = time.time()
                            is_tight_turn_counting_down = True
                            if turn_amount == 4 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                                traffic_light_area_2_list.append(is_closest_block_red)
                                last_traffic_light_time = time.time()
    elif is_traffic_adjusting:
        speed = 0.50
        if is_clockwise == None or is_clockwise == True:
            if last_is_closest_block_red:
                if is_traffic_adjusting_back:
                    heading_correction = 0

                    if abs(heading_error) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                        ideal_wall_distance_override = (1.0 - RED_WALL_DISTANCE_FROM_RIGHT) + 0.05
                        is_traffic_adjusting = False
                        is_traffic_adjusting_back = False
                elif abs(heading_error + TRAFFIC_HEADING_CORRECTION) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                    if back_ultrasonic * math.sin(math.radians(abs(heading_error))) >= (1.0 - RED_DISTANCE_FROM_RIGHT) + 0.05:
                        heading_correction = 0
                        is_traffic_adjusting_back = True
                    else:
                        heading_correction = TRAFFIC_HEADING_CORRECTION
                else:
                    heading_correction = TRAFFIC_HEADING_CORRECTION
            else:
                if is_traffic_adjusting_back:
                    heading_correction = 0

                    if abs(heading_error) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                        ideal_wall_distance_override = GREEN_WALL_DISTANCE_FROM_LEFT
                        is_traffic_adjusting = False
                        is_traffic_adjusting_back = False
                elif abs(heading_error - TRAFFIC_HEADING_CORRECTION) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                    if front_ultrasonic * math.sin(math.radians(abs(heading_error))) <= GREEN_DISTANCE_FROM_LEFT:
                        heading_correction = 0
                        is_traffic_adjusting_back = True
                    else:
                        heading_correction = -TRAFFIC_HEADING_CORRECTION
                else:
                    heading_correction = -TRAFFIC_HEADING_CORRECTION
        else:
            if last_is_closest_block_red:
                if is_traffic_adjusting_back:
                    heading_correction = 0

                    if abs(heading_error) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                        ideal_wall_distance_override = RED_WALL_DISTANCE_FROM_RIGHT
                        is_traffic_adjusting = False
                        is_traffic_adjusting_back = False
                elif abs(heading_error + TRAFFIC_HEADING_CORRECTION) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                    if front_ultrasonic * math.sin(math.radians(abs(heading_error))) <= RED_DISTANCE_FROM_RIGHT:
                        heading_correction = 0
                        is_traffic_adjusting_back = True
                    else:
                        heading_correction = TRAFFIC_HEADING_CORRECTION
                else:
                    heading_correction = TRAFFIC_HEADING_CORRECTION
            else:
                if is_traffic_adjusting_back:
                    heading_correction = 0

                    if abs(heading_error) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                        ideal_wall_distance_override = (1.0 - GREEN_WALL_DISTANCE_FROM_LEFT) + 0.05
                        is_traffic_adjusting = False
                        is_traffic_adjusting_back = False
                elif abs(heading_error - TRAFFIC_HEADING_CORRECTION) <= TRAFFIC_HEADING_ERROR_THRESHOLD:
                    if back_ultrasonic * math.sin(math.radians(abs(heading_error))) >= (1.0 - GREEN_DISTANCE_FROM_LEFT) + 0.05:
                        heading_correction = 0
                        is_traffic_adjusting_back = True
                    else:
                        heading_correction = -TRAFFIC_HEADING_CORRECTION
                else:
                    heading_correction = -TRAFFIC_HEADING_CORRECTION
    elif closest_block_x is not None:
        if is_clockwise is not None:
            if (is_clockwise and closest_block_x <= CAMERA_WIDTH*0.8) or (not is_clockwise and closest_block_x >= CAMERA_WIDTH*0.2):
                if turn_amount == 0 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                    traffic_light_area_1_list.append(is_closest_block_red)
                    last_traffic_light_time = time.time()
                if turn_amount == 4 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                    traffic_light_area_2_list.append(is_closest_block_red)
                    last_traffic_light_time = time.time()
        else:
            if turn_amount == 0 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                traffic_light_area_1_list.append(is_closest_block_red)
                last_traffic_light_time = time.time()
            if turn_amount == 4 and time.time() - last_traffic_light_time >= TRAFFIC_LIGHT_LIST_COOLDOWN_TIME:
                traffic_light_area_2_list.append(is_closest_block_red)
                last_traffic_light_time = time.time()

        if last_is_closest_block_red is None:
            if is_closest_block_red:
                heading_correction = TRAFFIC_HEADING_CORRECTION
            else:
                heading_correction = -TRAFFIC_HEADING_CORRECTION
            speed = 0.50
            is_traffic_adjusting = True
            last_is_closest_block_red = is_closest_block_red
        else:
            if not last_is_closest_block_red == is_closest_block_red:
                if is_closest_block_red:
                    heading_correction = TRAFFIC_HEADING_CORRECTION
                else:
                    heading_correction = -TRAFFIC_HEADING_CORRECTION
                speed = 0.50
                is_traffic_adjusting = True
                last_is_closest_block_red = is_closest_block_red
            else:
                if not is_last_is_closest_block_red_same:
                    is_last_is_closest_block_red_same = True
    else:
        if not front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_THRESHOLD:
            ultrasonic_below_threshold_time = time.time()
            is_ultrasonic_below_threshold = False
        elif time.time() - ultrasonic_below_threshold_time >= ULTRASONIC_TIME_THRESHOLD:
            if is_clockwise is None:
                is_ultrasonic_below_threshold = False
            else:
                if is_clockwise and heading_error <= ULTRASONIC_ANGLE_THRESHOLD:
                    is_ultrasonic_below_threshold = True
                elif not is_clockwise and heading_error >= -ULTRASONIC_ANGLE_THRESHOLD:
                    is_ultrasonic_below_threshold = True
                else:
                    is_ultrasonic_below_threshold = False
        
        if not front_ultrasonic * math.cos(math.radians(abs(heading_error))) < ULTRASONIC_THRESHOLD_2:
            ultrasonic_below_threshold_time_2 = time.time()
            is_ultrasonic_below_threshold_2 = False
        elif time.time() - ultrasonic_below_threshold_time_2 >= ULTRASONIC_TIME_THRESHOLD:
            if is_clockwise is None:
                is_ultrasonic_below_threshold_2 = False
            else:
                if is_clockwise and heading_error <= ULTRASONIC_ANGLE_THRESHOLD:
                    is_ultrasonic_below_threshold_2 = True
                elif not is_clockwise and heading_error >= -ULTRASONIC_ANGLE_THRESHOLD:
                    is_ultrasonic_below_threshold_2 = True
                else:
                    is_ultrasonic_below_threshold_2 = False

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

        if is_clockwise is not None:
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
                last_is_closest_block_red = None
                ideal_wall_distance_override = IDEAL_WALL_DISTANCE
                is_last_is_closest_block_red_same = False
                turn_amount += 1
            elif is_ultrasonic_below_threshold_2 and time.time() - last_turn_time > TURN_COOLDOWN_TIME_2 and last_is_closest_block_red is not None:
                if is_clockwise and last_is_closest_block_red:
                    is_tight_turn = True
                elif not is_clockwise and not last_is_closest_block_red:
                    is_tight_turn = True


        if is_clockwise is None:
            wall_error = (right_ultrasonic - left_ultrasonic) * math.cos(math.radians(abs(heading_error))) / 2.0
        else:
            wall_error = 0
            if is_clockwise:
                wall_error = -left_ultrasonic * math.cos(math.radians(abs(heading_error))) + ideal_wall_distance_override
            else:
                wall_error = right_ultrasonic * math.cos(math.radians(abs(heading_error))) - ideal_wall_distance_override
            
            heading_correction = wall_distance_pid.update(wall_error, delta_time)
            heading_correction = max(min(heading_correction, MAX_HEADING_ERROR), -MAX_HEADING_ERROR)
    
    heading_error = normalize_angle_error(heading_error + heading_correction)

    # Apply heading PID controller
    steering_adjustment = heading_pid.update(heading_error, delta_time)
    steering_percent = max(min(steering_adjustment, 1.00), -1.00)

    return speed, steering_percent

def process_image(image):
    # Convert image to HSV color space
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    # Create masks for blue and orange colors
    mask_blue = cv2.inRange(hsv_image, LOWER_BLUE_LINE, UPPER_BLUE_LINE)
    mask_orange = cv2.inRange(hsv_image, LOWER_ORANGE_LINE, UPPER_ORANGE_LINE)
    
    def get_line_properties(mask):
        coordinates = np.column_stack(np.where(mask > 0))
        # if coordinates.size == 0:
        #     return None, 0
        # average_y = int(np.mean(coordinates[:, 0]))

        # return average_y, coordinates.size
        return None, coordinates.size

    _, blue_line_size = get_line_properties(mask_blue)
    _, orange_line_size = get_line_properties(mask_orange)


    # Create masks for red and green colors
    mask_red1 = cv2.inRange(hsv_image, LOWER_RED1_LIGHT, UPPER_RED1_LIGHT)
    mask_red2 = cv2.inRange(hsv_image, LOWER_RED2_LIGHT, UPPER_RED2_LIGHT)
    mask_red = mask_red1 | mask_red2
    mask_green = cv2.inRange(hsv_image, LOWER_GREEN_LIGHT, UPPER_GREEN_LIGHT)
    
    # Find contours for red and green blocks
    contours_red, _ = cv2.findContours(mask_red, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours_green, _ = cv2.findContours(mask_green, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    
    def get_centroid_and_area(contour):
        M = cv2.moments(contour, True)
        if M["m00"] == 0:
            return None, 0
        cx = int(M["m10"] / M["m00"])
        cy = int(M["m01"] / M["m00"])
        area = cv2.contourArea(contour)
        return (cx, cy), area
    
    # Get centroids and areas for red and green blocks
    red_blocks = [get_centroid_and_area(c) for c in contours_red if get_centroid_and_area(c)[0] is not None]
    green_blocks = [get_centroid_and_area(c) for c in contours_green if get_centroid_and_area(c)[0] is not None]

    def find_closest_block(blocks):
        if not blocks:
            return None
        
        # Sort blocks by area (descending) and then by y-coordinate of the centroid (ascending)
        sorted_blocks = sorted(blocks, key=lambda b: (-b[1], b[0][1]))
        
        if sorted_blocks[0][1] < TRAFFIC_LIGHT_THRESHOLD: return None
        return sorted_blocks[0][0]
    
    closest_red_block = find_closest_block(red_blocks)
    closest_green_block = find_closest_block(green_blocks)
    
    def get_closest_block_to_camera(red_block, green_block):
        if red_block is None:
            return green_block, False
        elif green_block is None:
            return red_block, True
        
        # Compare areas and y-coordinates to find the closest block
        if red_block[1] < green_block[1]:
            return green_block, False
        elif red_block[1] > green_block[1]:
            return red_block, True
        else:
            if red_block[0][1] > green_block[0][1]:
                return green_block, False
            else:
                return red_block, True
    
    closest_block, is_closest_block_red = get_closest_block_to_camera(closest_red_block, closest_green_block)

    closest_block_x = None
    if closest_block is not None:
        closest_block_x = closest_block[0]
    
    return blue_line_size, orange_line_size, closest_block_x, is_closest_block_red