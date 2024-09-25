import time
from typing import Callable, List, Optional

def normalize_angle_error(angle: float) -> float:
    """
    Normalize an angle to the range [-180, 180].

    Args:
        angle (float): The angle to be normalized.

    Returns:
        float: The normalized angle.
    """
    return (angle + 180) % 360 - 180

# def execute_with_cooldown(condition: bool, cooldown_duration: float, last_executed_time: list[float], action: callable):
#     """
#     Executes the given action if the condition is True and the specified cooldown duration has passed since the last execution.
#     Returns the value returned by the action, or None if the action is not executed.

#     Args:
#         condition (bool): The condition that must be True for the action to execute.
#         cooldown_duration (float): The minimum time (in seconds) that must pass before the action can be executed again.
#         last_executed_time (list[float]): A list containing the timestamp of the last execution. The first element is used to store the last execution time.
#         action (callable): The function or action to be executed if the condition and cooldown criteria are met.

#     Returns:
#         The value returned by the action, or None if the action is not executed.
#     """
#     if condition and time.time() - last_executed_time[0] >= cooldown_duration:
#         result = action()
#         last_executed_time[0] = time.time()
#         return result
#     return None

# def execute_within_time_window(condition: bool, time_window: float, last_checked_time: list[float], action: callable):
#     """
#     Executes the given action if the condition is True and the specified time window has passed since the last check.
#     Returns the value returned by the action, or None if the action is not executed.

#     Args:
#         condition (bool): The condition that must be True for the action to execute.
#         time_window (float): The time window (in seconds) during which the action can be executed if the condition is met.
#         last_checked_time (list[float]): A list containing the timestamp of the last check. The first element is used to store the last check time.
#         action (callable): The function or action to be executed if the condition and time window criteria are met.

#     Returns:
#         The value returned by the action, or None if the action is not executed.
#     """
#     if condition and time.time() - last_checked_time[0] >= time_window:
#         return action()
#     else:
#         last_checked_time[0] = time.time()
#     return None

# def execute_with_linger(condition: bool, linger_duration: float, last_triggered_time: list[float], action: callable):
#     """
#     Executes the given action if the condition is True, and continues to execute it for a specified linger duration after the condition becomes False.
#     Returns the value returned by the action, or None if the action is not executed.

#     Args:
#         condition (bool): The condition that, when True, will update the last triggered time.
#         linger_duration (float): The duration (in seconds) during which the action continues to be executed after the condition becomes False.
#         last_triggered_time (list[float]): A list containing the timestamp of the last time the condition was True. The first element is used to store the last triggered time.
#         action (callable): The function or action to be executed as long as the condition is True or within the linger duration after it becomes False.

#     Returns:
#         The value returned by the action, or None if the action is not executed.
#     """
#     if condition:
#         last_triggered_time[0] = time.time()

#     if time.time() - last_triggered_time[0] <= linger_duration:
#         return action()
#     return None


def execute_with_timing_conditions(
    condition: bool,
    last_time_list: List[float],
    cooldown_duration: Optional[float] = None,
    time_window: Optional[float] = None,
    linger_duration: Optional[float] = None
) -> bool:
    """
    Determines whether the specified action should be executed based on various timing conditions.

    The function checks if the action should be executed based on the following conditions:
    1. The `condition` must be True.
    2. If `cooldown_duration` is provided, the time elapsed since the last execution 
       (tracked by `last_time_list[0]`) must meet or exceed this duration.
    3. If `time_window` is provided, the time elapsed since the last check 
       (tracked by `last_time_list[1]`) must meet or exceed this duration.
    4. If `linger_duration` is provided, the action continues to be considered for execution if:
       - The `condition` was True and the current time is within the linger duration 
         (tracked by `last_time_list[2]`) after the `condition` was last True.

    Args:
        condition (bool): A boolean value that must be True for the action to be considered for execution.
        last_time_list (List[float]): A list of timestamps used for timing checks:
            - `last_time_list[0]`: Timestamp of the last action execution.
            - `last_time_list[1]`: Timestamp of the last timing check.
            - `last_time_list[2]`: Timestamp when the condition was last True.
        cooldown_duration (Optional[float]): Minimum time (in seconds) that must pass 
            since the last execution before the action can be executed again. If None, 
            no cooldown restriction is applied.
        time_window (Optional[float]): Minimum time (in seconds) that must pass since 
            the last check before the action can be executed. If None, no time window 
            restriction is applied.
        linger_duration (Optional[float]): Duration (in seconds) for which the action 
            is still considered for execution after the condition becomes False. If None, 
            no lingering restriction is applied.

    Returns:
        bool: True if the all the timing condition is met otherwise, False.
    """

    if not len(last_time_list) == 3: raise ValueError("last_time_list must be the length of 3")
    
    current_time = time.time()

    is_all_timing_met = condition

    if condition:    
        if cooldown_duration is not None:
            is_all_timing_met = is_all_timing_met and (current_time - last_time_list[0] >= cooldown_duration)

        if time_window is not None:
            is_all_timing_met = is_all_timing_met and (current_time - last_time_list[1] >= time_window)
    else:
        if time_window is not None:
            last_time_list[1] = current_time

    if linger_duration is not None:
        if is_all_timing_met:
            last_time_list[2] = current_time

        if current_time - last_time_list[2] <= linger_duration:
            if cooldown_duration is not None:
                last_time_list[0] = current_time
            
            return True
    elif is_all_timing_met:
        if cooldown_duration is not None:
            last_time_list[0] = current_time
        
        return True
    
    return False