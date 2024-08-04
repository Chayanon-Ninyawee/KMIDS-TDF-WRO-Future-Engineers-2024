def normalize_angle_error(angle: float) -> float:
    """
    Normalize an angle to the range [-180, 180].

    Args:
        angle (float): The angle to be normalized.

    Returns:
        float: The normalized angle.
    """
    return (angle + 180) % 360 - 180