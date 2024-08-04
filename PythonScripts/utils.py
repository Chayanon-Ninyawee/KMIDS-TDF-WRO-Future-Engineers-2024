import numpy as np

def normalize_angle_error(angle: float) -> float:
    """
    Normalize an angle to the range [-180, 180].

    Args:
        angle (float): The angle to be normalized.

    Returns:
        float: The normalized angle.
    """
    return (angle + 180) % 360 - 180

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

def lowest_coordinate(mask):
    # Find lowest coordinates (highest value)
    coordinates = np.column_stack(np.where(mask > 0))
    if coordinates.size == 0:
        return None
    max_y = max(coordinates[:, 0])
    return int(max_y)