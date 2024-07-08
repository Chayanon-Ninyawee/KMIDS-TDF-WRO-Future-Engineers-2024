import numpy as np
import cv2
import struct
import math

import socketclient

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
    return average_y, coordinates.size

def main():
    socket_client = socketclient.SocketClient('127.0.0.1', 12345, 854, 480, 4*4)
    sock = socket_client._sock

    try:
        while True:
            socket_client.process_data()

            image = socket_client.get_image()

            hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV) 

            blue_line_mask = cv2.inRange(hsv, lower_blue_line, upper_blue_line)
            orange_line_mask = cv2.inRange(hsv, lower_orange_line, upper_orange_line)
            red_light_mask = cv2.inRange(hsv, lower_red_light, upper_red_light)
            green_light_mask = cv2.inRange(hsv, lower_green_light, upper_green_light)
                
            blue_line_y, blue_line_size = average_y_coordinate(blue_line_mask)
            orange_line_y, orange_line_size = average_y_coordinate(orange_line_mask)
            red_light_x, red_light_size = average_x_coordinate(red_light_mask)
            green_light_x, green_light_size = average_x_coordinate(green_light_mask)

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

            cv2.imshow('image', image)
            cv2.imshow('result', result)
            # cv2.imshow('blue_line_result', blue_line_result)
            # cv2.imshow('orange_line_result', orange_line_result) 
            # cv2.imshow('Received Image', result)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            message = struct.pack('f', 0.1) + struct.pack('f', 0.0)
            sock.send(message)
    except Exception as e: print(e)
    finally:
        cv2.destroyAllWindows()

if __name__ == "__main__":
    main()