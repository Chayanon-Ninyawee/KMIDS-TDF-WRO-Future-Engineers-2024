import cv2
import os
import re

from config import *

def process_image(image):
    hsv_image = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)

    # Create masks for blue and orange colors
    mask_blue = cv2.inRange(hsv_image, LOWER_BLUE_LINE, UPPER_BLUE_LINE)
    mask_orange = cv2.inRange(hsv_image, LOWER_ORANGE_LINE, UPPER_ORANGE_LINE)

    contours_blue, _ = cv2.findContours(mask_blue, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours_orange, _ = cv2.findContours(mask_orange, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    large_contours_blue = [c for c in contours_blue if cv2.contourArea(c) > MIN_BLUE_LINE_AREA]
    large_contours_orange = [c for c in contours_orange if cv2.contourArea(c) > MIN_ORANGE_LINE_AREA]

    mask_blue = np.zeros_like(mask_blue)
    mask_blue = cv2.drawContours(mask_blue, large_contours_blue, -1, 255, thickness=cv2.FILLED)

    mask_orange = np.zeros_like(mask_orange)
    mask_orange = cv2.drawContours(mask_orange, large_contours_orange, -1, 255, thickness=cv2.FILLED)


    mask_red1 = cv2.inRange(hsv_image, LOWER_RED1_LIGHT, UPPER_RED1_LIGHT)
    mask_red2 = cv2.inRange(hsv_image, LOWER_RED2_LIGHT, UPPER_RED2_LIGHT)
    mask_red = mask_red1 | mask_red2
    mask_green = cv2.inRange(hsv_image, LOWER_GREEN_LIGHT, UPPER_GREEN_LIGHT)

    contours_red, _ = cv2.findContours(mask_red, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    contours_green, _ = cv2.findContours(mask_green, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    large_contours_red = [c for c in contours_red if cv2.contourArea(c) > MIN_RED_LINE_AREA]
    large_contours_green = [c for c in contours_green if cv2.contourArea(c) > MIN_GREEN_LINE_AREA]

    mask_red = np.zeros_like(mask_red)
    mask_red = cv2.drawContours(mask_red, large_contours_red, -1, 255, thickness=cv2.FILLED)

    mask_green = np.zeros_like(mask_green)
    mask_green = cv2.drawContours(mask_green, large_contours_green, -1, 255, thickness=cv2.FILLED)


    mask_pink = cv2.inRange(hsv_image, LOWER_PINK_LIGHT, UPPER_PINK_LIGHT)

    blue_line_result = cv2.bitwise_and(image, image, mask = mask_blue)
    orange_line_result = cv2.bitwise_and(image, image, mask = mask_orange)
    red_light_result = cv2.bitwise_and(image, image, mask = mask_red)
    green_light_result = cv2.bitwise_and(image, image, mask = mask_green)
    pink_light_result = cv2.bitwise_and(image, image, mask = mask_pink)


    processed_image = green_light_result

    # processed_image = blue_line_result
    # processed_image = cv2.bitwise_or(processed_image, orange_line_result)
    # processed_image = cv2.bitwise_or(processed_image, red_light_result)
    # processed_image = cv2.bitwise_or(processed_image, green_light_result)
    # processed_image = cv2.bitwise_or(processed_image, pink_light_result)

    return processed_image


def extract_number(filename):
    # Extracts the number from the filename using regex
    match = re.search(r'(\d+)', filename)
    return int(match.group(0)) if match else float('inf')


def pick_color(event,x,y,flags,image_hsv):
    if event == cv2.EVENT_LBUTTONDOWN:
        pixel = image_hsv[y,x]
        print(pixel)

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
            cv2.setMouseCallback("image", pick_color, cv2.cvtColor(combined_image,cv2.COLOR_BGR2HSV))

            print(f"Displaying: {image_files[index]}")

            while True:
                key = cv2.waitKey(0)
                if key == ord('q'):
                    cv2.destroyAllWindows()
                    return
                elif key == ord('a'):
                    # Show the previous image
                    index = (index - 1) % len(image_files)
                    break
                elif key == ord('d'):
                    # Show the next image
                    index = (index + 1) % len(image_files)
                    break
        else:
            print(f"Failed to load image: {image_files[index]}")

# Example usage
# Specify the directory containing the images
image_directory = 'PythonScripts\\tools\\20240818_085957'

# Show the images from the specified directory
show_images_from_directory(image_directory)