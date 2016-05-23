// Written by Andrew Donelick
// adonelick@hmc.edu

#include <AttitudeController.h>
#include <RazorAHRS.h>
#include <SoftwareSerial.h>

// Pin which controls whether attitude control is on or off
#define ATTITUDE_CONTROL_PIN 4

// Actuation thresholds for each of the three axes
#define PITCH_THRESHOLD 0
#define ROLL_THRESHOLD 0
#define YAW_THRESHOLD 0

// Characters which determine what kind of data is coming
// over the serial line from the flight computer
#define DESIRED_YAW 'Y'
#define P_GAIN 'P'
#define I_GAIN 'I'
#define D_GAIN 'D'

SoftwareSerial mySerial(6, 5); // RX, TX
RazorAHRS razor(Serial);
AttitudeController attitudeController(100);

// Properties of the attitude controller
long proportionalGain = 50;
long integralGain = 0;
long derivativeGain = 20;

// Variables which hold the current and desired attitude values
long desiredYaw;
long pitch;
long roll;
long yaw;

void setup() {
  
    // Set up the attitude control system
    razor.begin();
    mySerial.begin(9600);
    desiredYaw = 0;
    pinMode(ATTITUDE_CONTROL_PIN, INPUT);
  
    // Set up the attitude controller
    attitudeController.setActuatorPins(PITCH, 8, 9);
    attitudeController.setActuatorPins(ROLL, 12, 13);
    attitudeController.setActuatorPins(YAW, 10, 11);
  
    attitudeController.setActuationThreshold(PITCH, PITCH_THRESHOLD);
    attitudeController.setActuationThreshold(ROLL, ROLL_THRESHOLD);
    attitudeController.setActuationThreshold(YAW, YAW_THRESHOLD);
  
    attitudeController.setGains(PITCH, 0, 0, 0);
    attitudeController.setGains(ROLL, 0, 0, 0);
    attitudeController.setGains(YAW, proportionalGain, integralGain, derivativeGain);
  
    attitudeController.setActuationThreshold(PITCH, 0);
    attitudeController.setActuationThreshold(ROLL, 0);
    attitudeController.setActuationThreshold(YAW, 0);
  
    attitudeController.setDesiredState(0, 0, 0);
    attitudeController.begin();
}

void loop() {
  
    // Process the information from the serial port connected
    // to the main flight computer
    processSerial();
    
    // Determine if we should be controlling the attitude
    // of the payload by reading the pin
    if (digitalRead(ATTITUDE_CONTROL_PIN)) {
      attitudeController.enable();
    } else {
      attitudeController.disable();
    }
  
    // Decode the most recent attitude data from the AHRS
    if (razor.available()) {
        razor.decodeMessage();
    }
  
    // Get the current state of the payload in hundredths of degrees
    pitch = 100*razor.getPitch();
    roll = 100*razor.getRoll();
    yaw = 100*razor.getYaw();
  
    // Use the attitude controller to control the payload's yaw
    // (right now we only control the yaw of the payload)
    attitudeController.updateState(pitch, roll, yaw, millis());
    attitudeController.updateActuators();
  
    delay(10);
}
