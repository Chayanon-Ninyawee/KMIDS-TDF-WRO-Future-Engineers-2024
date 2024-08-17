import cv2
import os
import re

from config import *

def process_image(image):
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Create masks for blue and orange colors
    mask_blue = cv2.inRange(hsv_image, LOWER_BLUE_LINE, UPPER_BLUE_LINE)
    mask_orange = cv2.inRange(hsv_image, LOWER_ORANGE_LINE, UPPER_ORANGE_LINE)

    mask_red1 = cv2.inRange(hsv_image, LOWER_RED1_LIGHT, UPPER_RED1_LIGHT)
    mask_red2 = cv2.inRange(hsv_image, LOWER_RED2_LIGHT, UPPER_RED2_LIGHT)
    mask_red = mask_red1 | mask_red2
    mask_green = cv2.inRange(hsv_image, LOWER_GREEN_LIGHT, UPPER_GREEN_LIGHT)

    mask_pink = cv2.inRange(hsv_image, LOWER_PINK_LIGHT, UPPER_PINK_LIGHT)

    blue_line_result = cv2.bitwise_and(image, image, mask = mask_blue)
    orange_line_result = cv2.bitwise_and(image, image, mask = mask_orange)
    red_light_result = cv2.bitwise_and(image, image, mask = mask_red)
    green_light_result = cv2.bitwise_and(image, image, mask = mask_green)
    pink_light_result = cv2.bitwise_and(image, image, mask = mask_pink)

    processed_image = cv2.bitwise_or(blue_line_result, orange_line_result)
    processed_image = cv2.bitwise_or(processed_image, red_light_result)
    processed_image = cv2.bitwise_or(processed_image, green_light_result)
    processed_image = cv2.bitwise_or(processed_image, pink_light_result)

    return processed_image


def extract_number(filename):
    # Extracts the number from the filename using regex
    match = re.search(r'(\d+)', filename)
    return int(match.group(0)) if match else float('inf')

def show_images_from_directory(directory):
    image_files = [f for f in os.listdir(directory) if f.lower().endswith(('.png'))]
    image_files.sort(key=extract_number)

    # Check if there are images to display
    if not image_files:
        print("No images found in the directory.")
        return

    # Initialize index to keep track of current image
    index = 0

    while True:
        # Construct the full path to the current image file
        filepath = os.path.join(directory, image_files[index])
        
        # Read the image
        image = cv2.imread(filepath)
        
        if image is not None:
            processed_image = process_image(image)
            
            spacer_height = image.shape[0]
            spacer_width = 50  # Width of the spacer
            white_spacer = 255 * np.ones((spacer_height, spacer_width, 3), dtype=np.uint8)

            combined_image = cv2.hconcat([image, white_spacer, processed_image])

            cv2.imshow('image', combined_image)
            print(f"Displaying: {image_files[index]}")

            while True:
                key = cv2.waitKey(0)
                if key == 113:  # Press q key to exit
                    cv2.destroyAllWindows()
                    return
                elif key == 81:  # Left arrow key (code 81)
                    # Show the previous image
                    index = (index - 1) % len(image_files)
                    break
                elif key == 83:  # Right arrow key (code 83)
                    # Show the next image
                    index = (index + 1) % len(image_files)
                    break
        else:
            print(f"Failed to load image: {image_files[index]}")

# Example usage
# Specify the directory containing the images
image_directory = 'PythonScripts/tools/20240817_161934'

# Show the images from the specified directory
show_images_from_directory(image_directory)