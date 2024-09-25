import cv2
import robotdata

from config import *

robot_data = robotdata.RobotData(CAMERA_WIDTH, CAMERA_HEIGHT)

robot_data.process_data()

full_image = robot_data.get_image()
image = full_image[IMAGE_HEIGHT:, :]

ultrasonic_info = robot_data.get_ultrasonic_data()
gyro_info = robot_data.get_gyro_data()

cv2.imwrite("full_image.png", full_image)
cv2.imwrite("cutted_image.png", image)

print(ultrasonic_info)
print(gyro_info)