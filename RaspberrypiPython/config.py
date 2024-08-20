import numpy as np

CAMERA_WIDTH = 854
CAMERA_HEIGHT = 480

IMAGE_HEIGHT = round(CAMERA_HEIGHT*0.46) # Less mean more

FRONT_BACK_ULTRASONIC_DISTANCE = 0.175
LEFT_RIGHT_ULTRASONIC_DISTANCE = 0.140

# Threshold of blue line in HSV space
# Measured blue line HSV [114 107 167]
LOWER_BLUE_LINE = np.array([99, 9, 45]) 
UPPER_BLUE_LINE = np.array([140, 230, 210])
MIN_BLUE_LINE_AREA = 100

# Threshold of orange line in HSV space
# Measured orange line HSV (11, 166, 231)
LOWER_ORANGE_LINE = np.array([8, 90, 160]) 
UPPER_ORANGE_LINE = np.array([21, 230, 255])
MIN_ORANGE_LINE_AREA = 100

# Threshold of red traffic light in HSV space
# Measured red traffic light HSV: 177, 213, 238
LOWER_RED1_LIGHT = np.array([179, 110, 120]) 
UPPER_RED1_LIGHT = np.array([180, 240, 250])
LOWER_RED2_LIGHT = np.array([0, 110, 120]) 
UPPER_RED2_LIGHT = np.array([6, 240, 250])
MIN_RED_LINE_AREA = 200

# Threshold of green traffic light in HSV space
# Measured green traffic light HSV: 56, 202, 214
LOWER_GREEN_LIGHT = np.array([36, 100, 90]) 
UPPER_GREEN_LIGHT = np.array([47, 210, 215])
MIN_GREEN_LINE_AREA = 200

# Threshold of green traffic light in HSV space
# Measured green traffic light HSV: 150, 218, 196
LOWER_PINK_LIGHT = np.array([169, 201, 110]) 
UPPER_PINK_LIGHT = np.array([175, 241, 250])