import cv2
import datetime
import time
from picamera2 import Picamera2
from libcamera import controls

from config import *

# Initialize the Picamera2
picam2 = Picamera2()

# Configure the camera
config = picam2.create_preview_configuration(main={"size": (854, 480)})
picam2.configure(config)

picam2.set_controls({"AfMode":controls.AfModeEnum.Manual, "LensPosition": 0.5})
picam2.set_controls({"AwbEnable": False, "Brightness": 0.1})
picam2.set_controls({"HdrMode": controls.HdrModeEnum.Off})

# Start the camera
picam2.start()

time.sleep(1)

try:
    # Capture a frame
    image = picam2.capture_array()
    image = cv2.rotate(image, cv2.ROTATE_180)
    image = cv2.cvtColor(image, cv2.COLOR_RGB2BGR)

    # Convert image to HSV color space
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Create masks for blue and orange colors
    mask_blue = cv2.inRange(hsv_image, LOWER_BLUE_LINE, UPPER_BLUE_LINE)
    mask_orange = cv2.inRange(hsv_image, LOWER_ORANGE_LINE, UPPER_ORANGE_LINE)

    mask_red1 = cv2.inRange(hsv_image, LOWER_RED1_LIGHT, UPPER_RED1_LIGHT)
    mask_red2 = cv2.inRange(hsv_image, LOWER_RED2_LIGHT, UPPER_RED2_LIGHT)
    mask_red = mask_red1 | mask_red2
    mask_green = cv2.inRange(hsv_image, LOWER_GREEN_LIGHT, UPPER_GREEN_LIGHT)
    
    time_string = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")

    filename = time_string + ".png"
    cv2.imwrite(filename, image)
    print(f"Image saved as {filename}")

    blue_line_result = cv2.bitwise_and(image, image, mask = mask_blue)
    orange_line_result = cv2.bitwise_and(image, image, mask = mask_orange)
    red_light_result = cv2.bitwise_and(image, image, mask = mask_red)
    green_light_result = cv2.bitwise_and(image, image, mask = mask_green)

    processed_image = cv2.bitwise_or(blue_line_result, orange_line_result)
    processed_image = cv2.bitwise_or(processed_image, red_light_result)
    processed_image = cv2.bitwise_or(processed_image, green_light_result)

    processed_filename = time_string + "-processed.png"
    cv2.imwrite(processed_filename, processed_image)
    print(f"Image saved as {processed_filename}")
finally:
    # Stop the camera
    picam2.stop()
    
    # Destroy all OpenCV windows
    cv2.destroyAllWindows()