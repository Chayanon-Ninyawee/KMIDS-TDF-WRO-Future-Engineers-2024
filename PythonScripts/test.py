import cv2
import numpy as np

def process_image(image):
    # Convert image to HSV color space
    hsv = cv2.cvtColor(image, cv2.COLOR_BGR2HSV)
    
    # Define color ranges in HSV
    # Red range (lower and upper bounds)
    lower_red1 = np.array([0, 100, 100])
    upper_red1 = np.array([10, 255, 255])
    lower_red2 = np.array([160, 100, 100])
    upper_red2 = np.array([180, 255, 255])
    
    # Green range
    lower_green = np.array([35, 100, 100])
    upper_green = np.array([85, 255, 255])
    
    # Create masks for red and green colors
    mask_red1 = cv2.inRange(hsv, lower_red1, upper_red1)
    mask_red2 = cv2.inRange(hsv, lower_red2, upper_red2)
    mask_red = mask_red1 | mask_red2
    mask_green = cv2.inRange(hsv, lower_green, upper_green)
    
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
        
        return sorted_blocks[0][0]
    
    closest_red_block = find_closest_block(red_blocks)
    closest_green_block = find_closest_block(green_blocks)
    
    def get_closest_block_to_camera(block1, block2):
        if block1 is None:
            return block2
        if block2 is None:
            return block1
        # Compare areas and y-coordinates to find the closest block
        if block1[1] > block2[1]:
            return block1[0]
        elif block1[1] < block2[1]:
            return block2[0]
        else:
            return block1[0] if block1[0][1] < block2[0][1] else block2[0]
    
    closest_block = get_closest_block_to_camera(closest_red_block, closest_green_block)
    
    return closest_block

def display_closest_block(image, closest_block):
    # Check if a closest block is found
    if closest_block:
        # Draw a circle at the centroid of the closest block
        cv2.circle(image, closest_block, 10, (0, 255, 0), -1)  # Green circle
        
        # Display the image
        cv2.imshow('Closest Block', image)
    else:
        cv2.imshow('Closest Block', image)

    cv2.waitKey(1) & 0xFF == ord('q')

# Example usage
closest_block = process_image(image)
display_closest_block(image, closest_block)