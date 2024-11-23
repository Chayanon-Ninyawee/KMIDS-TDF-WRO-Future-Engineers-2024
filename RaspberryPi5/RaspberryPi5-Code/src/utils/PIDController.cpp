class PIDController {
public:
    PIDController(float kp, float ki, float kd) 
        : kp(kp), ki(ki), kd(kd), prevError(0.0f), integral(0.0f) {}

    float calculate(float error, float deltaTime) {
        integral += error * deltaTime;
        float derivative = (error - prevError) / deltaTime;
        prevError = error;

        return kp * error + ki * integral + kd * derivative;
    }

private:
    float kp;         // Proportional coefficient
    float ki;         // Integral coefficient
    float kd;         // Derivative coefficient
    float prevError;  // Previous error
    float integral;   // Accumulated integral
};
