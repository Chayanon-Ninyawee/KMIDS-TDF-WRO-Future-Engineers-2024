import cv2
import math
import time

import pidcontroller

from config import *


suggested_heading = 0

ideal_wall_distance = 0.42

heading_pid = pidcontroller.PIDController(kp=0.1, ki=0.0, kd=0.01)
wall_distance_pid = pidcontroller.PIDController(kp=150.0, ki=0.0, kd=0)

is_clockwise = None
last_turn_time = 0

front_ultrasonic_list = [10,10,10,10,10,10,10,10,10,10]

ideal_wall_distance_overide = ideal_wall_distance
ideal_wall_distance_overide_distace_from_center = 0.2
ideal_wall_distance_overide_duration = 1.6
ideal_wall_distance_overide_last_time = 0

last_red_time = 0
last_green_time = 0
last_traffic_duration = 0.8

def process_data_obstacle(ultrasonic_info: tuple[int, int, int, int],
                gyro_info: float,
                blue_line_info: tuple[cv2.typing.MatLike, int, int],
                orange_line_info: tuple[cv2.typing.MatLike, int, int],
                red_light_info: tuple[cv2.typing.MatLike, int, int],
                green_light_info: tuple[cv2.typing.MatLike, int, int],
                delta_time: float
    ) -> tuple[float, float]:

    global suggested_heading
    global ideal_wall_distance
    global heading_pid, heading_pid
    global is_clockwise, last_turn_time
    global front_ultrasonic_list
    global ideal_wall_distance_overide, ideal_wall_distance_overide_distace_from_center, ideal_wall_distance_overide_duration, ideal_wall_distance_overide_last_time
    global last_red_time, last_green_time, last_traffic_durations

    front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info

    blue_line_mask, blue_line_y, blue_line_size = blue_line_info
    orange_line_mask, orange_line_y, orange_line_size = orange_line_info
    red_light_mask, red_light_x, red_light_size = red_light_info
    green_light_mask, green_light_x, green_light_size = green_light_info

    heading_error = gyro_info - suggested_heading
    heading_error = (heading_error + 180) % 360 - 180

    heading_error_copy = heading_error

    front_ultrasonic_value = front_ultrasonic*math.cos(math.radians(abs(heading_error)))
    if not is_clockwise == None:
        if is_clockwise:
            if heading_error > 10.0: front_ultrasonic_value = 10
        else:
            if heading_error < -10.0: front_ultrasonic_value = 10
    front_ultrasonic_list.append(front_ultrasonic_value)
    front_ultrasonic_list = front_ultrasonic_list[-10:]

    ideal_wall_distance = ideal_wall_distance
    if is_clockwise == None:
        if not (blue_line_size == None or orange_line_size == None):
            if blue_line_size - orange_line_size > 1000: is_clockwise = False
            elif orange_line_size - blue_line_size > 1000: is_clockwise = True
    else:
        if all(x < 0.9 for x in front_ultrasonic_list) and time.time() - last_turn_time > 4.5:
            if is_clockwise: suggested_heading -= 90
            else: suggested_heading += 90
            suggested_heading = suggested_heading % 360
            last_turn_time = time.time()
        
        if not time.time() - ideal_wall_distance_overide_last_time > ideal_wall_distance_overide_duration:
            if red_light_size == None: red_light_size = 0
            if green_light_size == None: green_light_size = 0
            if is_clockwise:
                if red_light_size > green_light_size:
                    if not time.time() - last_green_time < last_traffic_duration:
                        ideal_wall_distance += ideal_wall_distance_overide_distace_from_center
                        last_red_time = time.time()
                else:
                    if not time.time() - last_red_time < last_traffic_duration:
                        ideal_wall_distance -= ideal_wall_distance_overide_distace_from_center
                        last_green_time = time.time()
            else:
                if red_light_size > green_light_size:
                    if not time.time() - last_green_time < last_traffic_duration:
                        ideal_wall_distance -= ideal_wall_distance_overide_distace_from_center
                        last_red_time = time.time()
                else:
                    if not time.time() - last_red_time < last_traffic_duration:
                        ideal_wall_distance += ideal_wall_distance_overide_distace_from_center
                        last_green_time = time.time()

    wall_error = 0
    if is_clockwise == None:
        wall_error = (right_ultrasonic-left_ultrasonic)*math.cos(math.radians(abs(heading_error))) / 2.0
    else:
        if is_clockwise:
            wall_error = -left_ultrasonic*math.cos(math.radians(abs(heading_error))) + ideal_wall_distance
        else:
            wall_error = right_ultrasonic*math.cos(math.radians(abs(heading_error))) - ideal_wall_distance

    heading_correction = wall_distance_pid.update(wall_error, delta_time)

    if red_light_size == None: red_light_size = 0
    if green_light_size == None: green_light_size = 0
    if red_light_size > green_light_size:
        if red_light_size >= 2000:
            ideal_wall_distance_overide_last_time = time.time()
            heading_correction += (-red_light_x + (camera_width/2)*max(red_light_size/2000.0, 1.9))/camera_width*30.0
    else:
        if green_light_size >= 2000:
            ideal_wall_distance_overide_last_time = time.time()
            heading_correction += (green_light_x - (camera_width/2)*max(green_light_size/2000.0, 1.9))/camera_width*30.0
    heading_correction = max(min(heading_correction, 35.0), -35.0)



    heading_error += heading_correction
    heading_error = (heading_error + 180) % 360 - 180

    steering_adjustment = heading_pid.update(heading_error, delta_time)

    steering_percent = max(min(steering_adjustment, 1.00), -1.00)

    return 0.33, steering_percent