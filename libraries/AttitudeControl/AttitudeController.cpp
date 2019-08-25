// Written by Andrew Donelick
// adonelick@hmc.edu

#include "AttitudeController.h"

AttitudeController::AttitudeController(uint32_t waitTime)
    : waitTime_(waitTime),
      lastUpdateTime_(0),
      enabled_(false),
      numPoints_(0),
      startIndex_(0),
      endIndex_(POINTS_TO_STORE - 1)
{
    // Nothing to do here...
}


void AttitudeController::begin()
{
    // Sets all the actuator pins as outputs, turns all actuators off
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            pinMode(pins_[i][j], OUTPUT);
            digitalWrite(pins_[i][j], LOW);
        }
    }

    integralError_[PITCH] = 0;
    integralError_[ROLL] = 0;
    integralError_[YAW] = 0;
}


void AttitudeController::enable()
{
    enabled_ = true;
}


bool AttitudeController::enabled()
{
    return enabled_;
}

void AttitudeController::disable()
{

    if (!enabled_) {
        return;
    }

    enabled_ = false;

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            digitalWrite(pins_[i][j], LOW);
        }
    }

    // Reset some of the correction errors to zero
    integralError_[PITCH] = 0;
    integralError_[ROLL] = 0;
    integralError_[YAW] = 0;

    derivativeError_[PITCH] = 0;
    derivativeError_[ROLL] = 0;
    derivativeError_[YAW] = 0;
}


void AttitudeController::setActuatorPins(uint8_t axis, uint8_t plus, uint8_t minus)
{
    pins_[axis][PLUS] = plus;
    pins_[axis][MINUS] = minus;
}


void AttitudeController::setActuationThreshold(uint8_t axis, int32_t threshold)
{
    thresholds_[axis] = threshold;
}


int32_t AttitudeController::getActuation(uint8_t axis)
{
    int32_t p = p_gain_[axis];
    int32_t i = i_gain_[axis];
    int32_t d = d_gain_[axis];

    // Calculate the actuation from the compensator
    int32_t actuation = 0;
    if (p != 0) {
        actuation += proportionalError_[axis][endIndex_]/p;
    }

    if (i != 0) {
        actuation += integralError_[axis]/i;
    }

    if (d != 0) {
        actuation += derivativeError_[axis]/d;
    }
    
    // Limit the actuation to that available for the actuators
    if (actuation > MAX_ACTUATION) {
        actuation = MAX_ACTUATION;
    } else if (actuation < -MAX_ACTUATION) {
        actuation = -MAX_ACTUATION;
    }

    return actuation;
}


void AttitudeController::setGains(uint8_t axis, int32_t p, int32_t i, int32_t d)
{
    p_gain_[axis] = p;
    i_gain_[axis] = i;
    d_gain_[axis] = d;
}


void AttitudeController::updateState(int32_t pitch, int32_t roll, int32_t yaw, uint32_t newTime)
{
    // Increment the counter of the number of points stored
    // Shift the start and end indices of the ring buffers
    if (numPoints_ < POINTS_TO_STORE) {
        ++numPoints_;
        endIndex_ = (endIndex_ + 1) % POINTS_TO_STORE;
    } else {
        endIndex_ = (endIndex_ + 1) % POINTS_TO_STORE;
        startIndex_ = (startIndex_ + 1) % POINTS_TO_STORE;
    }

    // Save the most recent attitue and time reading
    actualState_[PITCH] = normalizeAngle(pitch);
    actualState_[ROLL] = normalizeAngle(roll);
    actualState_[YAW] = normalizeAngle(yaw);
    time_[endIndex_] = (int32_t) newTime;
    updateErrors();
}


void AttitudeController::setDesiredState(int32_t pitch, int32_t roll, int32_t yaw)
{
    desiredState_[PITCH] = normalizeAngle(pitch);
    desiredState_[ROLL] = normalizeAngle(roll);
    desiredState_[YAW] = normalizeAngle(yaw);
}


void AttitudeController::updateActuators()
{
    // Based on the current actuation signal, command the attitude
    // actuators to do what they need to do

    if (!enabled_) {
        return;
    }

    for (uint8_t axis = 0; axis < 3; ++axis) {
        int32_t actuation = getActuation(axis);

        // If we have passed enough time from the last actuation,
        // go ahead and update the actuators
        if (millis() - lastUpdateTime_ > waitTime_) {
            
            if (abs(actuation) >= thresholds_[axis]) {

                // If we are above the threshold, turn on 
                // the correct pin, and the other turn off
                if (actuation > 0) {
                    analogWrite(pins_[axis][PLUS], abs(actuation));
                    analogWrite(pins_[axis][MINUS], 0);   
                } else {
                    analogWrite(pins_[axis][PLUS], 0);
                    analogWrite(pins_[axis][MINUS], abs(actuation));
                }
                
            } else {
                // Otherwise, turn both pins for the axis off
                analogWrite(pins_[axis][PLUS], 0);
                analogWrite(pins_[axis][MINUS], 0); 
            }
        }
    }
}


void AttitudeController::updateErrors()
{
    // Set the error to be the difference between the most recent 
    // state reading and the desired state
    int32_t pitchError = desiredState_[PITCH] - actualState_[PITCH];
    int32_t rollError = desiredState_[ROLL] - actualState_[ROLL];
    int32_t yawError = desiredState_[YAW] - actualState_[YAW];
    int32_t dt = time_[endIndex_] - time_[(endIndex_ - 1) % POINTS_TO_STORE];

    proportionalError_[PITCH][endIndex_] = pitchError;
    proportionalError_[ROLL][endIndex_] = rollError;
    proportionalError_[YAW][endIndex_] = yawError;

    // Prevent integrator wind-up by returning if we are not actually 
    // controlling the payload's attitude
    if (!enabled_) {
        return;
    }

    // Calculate the integral of the error
    integralError_[PITCH] += (pitchError*dt) / 1000;
    integralError_[ROLL] += (rollError*dt) / 1000;
    integralError_[YAW] += (yawError*dt) / 1000;

    // Limit the integration value
    for (uint8_t axis = 0; axis < 3; ++axis) {
        if (integralError_[axis] > MAX_INTEGRAL) {
            integralError_[axis] = MAX_INTEGRAL;
        } else if (integralError_[axis] < -MAX_INTEGRAL) {
            integralError_[axis] = -MAX_INTEGRAL;
        }
    }

    // Calculate the derivative of the error (with noise reduction)
    derivativeError_[PITCH] = calculateSlope(time_, proportionalError_[PITCH]);
    derivativeError_[ROLL] = calculateSlope(time_, proportionalError_[ROLL]);
    derivativeError_[YAW] = calculateSlope(time_, proportionalError_[YAW]);
}


int32_t AttitudeController::normalizeAngle(int32_t angle)
{
    angle = angle % 36000;

    if (angle > 18000) {
        return angle - 36000;
    } else if (angle < -18000) {
        return angle + 36000;
    } else {
        return angle;
    }
}


int32_t AttitudeController::calculateSlope(int32_t x[], int32_t y[])
{
    // Use the following page to calculate the slope of a line
    // fit to the points contained in the arrays x, y
    // https://en.wikipedia.org/wiki/Simple_linear_regression#Fitting_the_regression_line

    // Calculate the mean of the x and y points
    int32_t meanX = mean(x);
    int32_t meanY = mean(y);

    // Calculate the numerator
    int32_t numerator = 0;
    for (uint16_t i = 0; i < numPoints_; ++i) {
        numerator += (x[i] - meanX)*(y[i] - meanY);
    }
    numerator *= 1000;

    // Calculate the denominator
    int32_t denominator = 0;
    for (uint16_t i = 0; i < numPoints_; ++i) {
        denominator += (x[i] - meanX)*(x[i] - meanX);
    }

    return numerator / denominator;
}


int32_t AttitudeController::mean(int32_t array[])
{
    if (numPoints_ == 0) {
        return 0;
    }

    // Calculate the integer mean of an array
    int32_t sum = 0;
    for (uint16_t i = 0; i < numPoints_; ++i) {
        sum += array[i];
    }

    return sum / numPoints_;
}
