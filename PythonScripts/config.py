import numpy as np

CAMERA_WIDTH = 854
CAMERA_HEIGHT = 480

IMAGE_HEIGHT = round(CAMERA_HEIGHT*0.49)

FRONT_BACK_ULTRASONIC_DISTANCE = 0.185
LEFT_RIGHT_ULTRASONIC_DISTANCE = 0.120

# Threshold of blue line in HSV space
# Measured blue line HSV: 111, 198, 165
LOWER_BLUE_LINE = np.array([106, 188, 155]) 
UPPER_BLUE_LINE = np.array([116, 208, 175])

# Threshold of orange line in HSV space
# Measured orange line HSV: 12, 232, 244
LOWER_ORANGE_LINE = np.array([7, 222, 234]) 
UPPER_ORANGE_LINE = np.array([17, 242, 254])

# Threshold of red traffic light in HSV space
# Measured red traffic light HSV: 177, 213, 238
LOWER_RED1_LIGHT = np.array([172, 203, 70]) 
UPPER_RED1_LIGHT = np.array([180, 223, 255])
LOWER_RED2_LIGHT = np.array([0, 203, 70]) 
UPPER_RED2_LIGHT = np.array([2, 223, 255])

# Threshold of green traffic light in HSV space
# Measured green traffic light HSV: 56, 202, 214
LOWER_GREEN_LIGHT = np.array([51, 192, 70]) 
UPPER_GREEN_LIGHT = np.array([61, 212, 224])

# Threshold of green traffic light in HSV space
# Measured green traffic light HSV: 150, 218, 196
LOWER_PINK_LIGHT = np.array([145, 198, 20]) 
UPPER_PINK_LIGHT = np.array([155, 252, 244])