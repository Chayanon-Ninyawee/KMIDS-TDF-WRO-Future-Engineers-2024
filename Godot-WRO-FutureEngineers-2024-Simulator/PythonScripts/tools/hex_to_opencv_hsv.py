import colorsys

def hex_to_rgb(hex_color):
    """
    Convert HEX color to RGB.
    HEX format: "#RRGGBB" or "RRGGBB"
    """
    hex_color = hex_color.lstrip('#')
    if len(hex_color) != 6:
        raise ValueError("Input should be a HEX color string of format #RRGGBB or RRGGBB")
    r = int(hex_color[0:2], 16)
    g = int(hex_color[2:4], 16)
    b = int(hex_color[4:6], 16)
    return (r, g, b)

def rgb_to_hsv_scaled(r, g, b):
    """
    Convert RGB to HSV with scaled output.
    RGB values should be in the range [0, 255].
    H will be in the range [0, 180].
    S and V will be in the range [0, 255].
    """
    r_scaled, g_scaled, b_scaled = r / 255.0, g / 255.0, b / 255.0
    h, s, v = colorsys.rgb_to_hsv(r_scaled, g_scaled, b_scaled)
    
    # Scale H to [0, 180], S and V to [0, 255]
    h_scaled = int(h * 180)
    s_scaled = int(s * 255)
    v_scaled = int(v * 255)
    return h_scaled, s_scaled, v_scaled

def hex_to_hsv_scaled(hex_color):
    """
    Convert HEX color to HSV with scaled output.
    """
    rgb = hex_to_rgb(hex_color)
    hsv = rgb_to_hsv_scaled(*rgb)
    return hsv

# Example usage
hex_color = "#cf7343"
hsv_color = hex_to_hsv_scaled(hex_color)
print(f"HEX color {hex_color} converts to HSV {hsv_color}")