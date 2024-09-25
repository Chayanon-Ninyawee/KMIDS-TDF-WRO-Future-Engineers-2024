class PIDController:
    def __init__(self, kp, ki, kd):
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.previous_error = 0
        self.integral = 0

    def update(self, error, delta_time):
        # Proportional term
        p = self.kp * error
        
        # Integral term
        self.integral += error * delta_time
        i = self.ki * self.integral
        
        # Derivative term
        if delta_time > 0:
            derivative = (error - self.previous_error) / delta_time
        else:
            derivative = 0
        d = self.kd * derivative
        
        # Save current error for next derivative calculation
        self.previous_error = error
        
        # Return the PID output
        return p + i + d