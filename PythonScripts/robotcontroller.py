import numpy as np
import cv2
import math
import time

import socketclient
import pidcontroller


camera_width = 854
camera_height = 480

robot_data = socketclient.SocketClient('127.0.0.1', 12345, camera_width, camera_height, 4*4, 4)


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

class OpenChallange:
    suggested_heading = 0

    ideal_wall_distance = 0.38

    heading_pid = pidcontroller.PIDController(kp=0.1, ki=0.0, kd=0.01)
    wall_distance_pid = pidcontroller.PIDController(kp=150.0, ki=0.0, kd=0)

    is_clockwise = None
    last_turn_time = 0
    
    @classmethod
    def process_data_open(cls,
                    ultrasonic_info: tuple[int, int, int, int],
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
        front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info

        blue_line_mask, blue_line_y, blue_line_size = blue_line_info
        orange_line_mask, orange_line_y, orange_line_size = orange_line_info

        heading_error = gyro_info - cls.suggested_heading
        heading_error = (heading_error + 180) % 360 - 180

        if cls.is_clockwise == None:
            if not (blue_line_size == None or orange_line_size == None):
                if blue_line_size - orange_line_size > 1000: cls.is_clockwise = False
                elif orange_line_size - blue_line_size > 1000: cls.is_clockwise = True
        else:
            if front_ultrasonic*math.cos(math.radians(abs(heading_error))) < 0.6 and time.time() - cls.last_turn_time > 3:
                if cls.is_clockwise: cls.suggested_heading -= 90
                else: cls.suggested_heading += 90
                cls.suggested_heading = cls.suggested_heading % 360
                cls.last_turn_time = time.time()

        wall_error = 0
        if cls.is_clockwise == None:
            wall_error = (right_ultrasonic-left_ultrasonic)*math.cos(math.radians(abs(heading_error))) / 2.0
        else:
            if cls.is_clockwise:
                wall_error = -left_ultrasonic*math.cos(math.radians(abs(heading_error))) + cls.ideal_wall_distance
            else:
                wall_error = right_ultrasonic*math.cos(math.radians(abs(heading_error))) - cls.ideal_wall_distance

        heading_correction = cls.wall_distance_pid.update(wall_error, delta_time)
        heading_correction = max(min(heading_correction, 30.0), -30.0)



        heading_error += heading_correction
        heading_error = (heading_error + 180) % 360 - 180

        steering_adjustment = cls.heading_pid.update(heading_error, delta_time)

        steering_percent = max(min(steering_adjustment, 1.00), -1.00)

        print(heading_correction)
        return 0.33, steering_percent

class ObstacleChallenge:
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
    
    @classmethod
    def process_data_obstacle(cls,
                    ultrasonic_info: tuple[int, int, int, int],
                    gyro_info: float,
                    blue_line_info: tuple[cv2.typing.MatLike, int, int],
                    orange_line_info: tuple[cv2.typing.MatLike, int, int],
                    red_light_info: tuple[cv2.typing.MatLike, int, int],
                    green_light_info: tuple[cv2.typing.MatLike, int, int],
                    delta_time: float
        ) -> tuple[float, float]:
        front_ultrasonic, back_ultrasonic, left_ultrasonic, right_ultrasonic = ultrasonic_info

        blue_line_mask, blue_line_y, blue_line_size = blue_line_info
        orange_line_mask, orange_line_y, orange_line_size = orange_line_info
        red_light_mask, red_light_x, red_light_size = red_light_info
        green_light_mask, green_light_x, green_light_size = green_light_info

        heading_error = gyro_info - cls.suggested_heading
        heading_error = (heading_error + 180) % 360 - 180

        heading_error_copy = heading_error

        front_ultrasonic_value = front_ultrasonic*math.cos(math.radians(abs(heading_error)))
        if not cls.is_clockwise == None:
            if cls.is_clockwise:
                if heading_error > 10.0: front_ultrasonic_value = 10
            else:
                if heading_error < -10.0: front_ultrasonic_value = 10
        cls.front_ultrasonic_list.append(front_ultrasonic_value)
        cls.front_ultrasonic_list = cls.front_ultrasonic_list[-10:]

        ideal_wall_distance = cls.ideal_wall_distance
        if cls.is_clockwise == None:
            if not (blue_line_size == None or orange_line_size == None):
                if blue_line_size - orange_line_size > 1000: cls.is_clockwise = False
                elif orange_line_size - blue_line_size > 1000: cls.is_clockwise = True
        else:
            if all(x < 0.9 for x in cls.front_ultrasonic_list) and time.time() - cls.last_turn_time > 4.5:
                if cls.is_clockwise: cls.suggested_heading -= 90
                else: cls.suggested_heading += 90
                cls.suggested_heading = cls.suggested_heading % 360
                cls.last_turn_time = time.time()
            
            if not time.time() - cls.ideal_wall_distance_overide_last_time > cls.ideal_wall_distance_overide_duration:
                if red_light_size == None: red_light_size = 0
                if green_light_size == None: green_light_size = 0
                if cls.is_clockwise:
                    if red_light_size > green_light_size:
                        if not time.time() - cls.last_green_time < cls.last_traffic_duration:
                            ideal_wall_distance += cls.ideal_wall_distance_overide_distace_from_center
                            cls.last_red_time = time.time()
                    else:
                        if not time.time() - cls.last_red_time < cls.last_traffic_duration:
                            ideal_wall_distance -= cls.ideal_wall_distance_overide_distace_from_center
                            cls.last_green_time = time.time()
                else:
                    if red_light_size > green_light_size:
                        if not time.time() - cls.last_green_time < cls.last_traffic_duration:
                            ideal_wall_distance -= cls.ideal_wall_distance_overide_distace_from_center
                            cls.last_red_time = time.time()
                    else:
                        if not time.time() - cls.last_red_time < cls.last_traffic_duration:
                            ideal_wall_distance += cls.ideal_wall_distance_overide_distace_from_center
                            cls.last_green_time = time.time()

        wall_error = 0
        if cls.is_clockwise == None:
            wall_error = (right_ultrasonic-left_ultrasonic)*math.cos(math.radians(abs(heading_error))) / 2.0
        else:
            if cls.is_clockwise:
                wall_error = -left_ultrasonic*math.cos(math.radians(abs(heading_error))) + ideal_wall_distance
            else:
                wall_error = right_ultrasonic*math.cos(math.radians(abs(heading_error))) - ideal_wall_distance

        heading_correction = cls.wall_distance_pid.update(wall_error, delta_time)

        if red_light_size == None: red_light_size = 0
        if green_light_size == None: green_light_size = 0
        if red_light_size > green_light_size:
            if red_light_size >= 2000:
                cls.ideal_wall_distance_overide_last_time = time.time()
                heading_correction += (-red_light_x + (camera_width/2)*max(red_light_size/2000.0, 1.9))/camera_width*30.0
        else:
            if green_light_size >= 2000:
                cls.ideal_wall_distance_overide_last_time = time.time()
                heading_correction += (green_light_x - (camera_width/2)*max(green_light_size/2000.0, 1.9))/camera_width*30.0
        heading_correction = max(min(heading_correction, 35.0), -35.0)



        heading_error += heading_correction
        heading_error = (heading_error + 180) % 360 - 180

        steering_adjustment = cls.heading_pid.update(heading_error, delta_time)

        steering_percent = max(min(steering_adjustment, 1.00), -1.00)

        return 0.33, steering_percent


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
            # speed_target, steering_percent = OpenChallange.process_data_open(ultrasonic_info, gyro_info, blue_line_info, orange_line_info, red_light_info, green_light_info, delta_time)
            speed_target, steering_percent = ObstacleChallenge.process_data_obstacle(ultrasonic_info, gyro_info, blue_line_info, orange_line_info, red_light_info, green_light_info, delta_time)

            robot_data.send_data(speed_target, steering_percent)

            if show_window(image, blue_line_info, orange_line_info, red_light_info, green_light_info): break
    except Exception as e: print(e)
    finally:
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()