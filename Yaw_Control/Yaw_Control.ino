#include <AttitudeController.h>
#include <RazorAHRS.h>
#include <SoftwareSerial.h>

#define ATTITUDE_CONTROL_PIN 4

#define PITCH_THRESHOLD 0
#define ROLL_THRESHOLD 0
#define YAW_THRESHOLD 0

SoftwareSerial mySerial(6, 5); // RX, TX
RazorAHRS razor(Serial);
AttitudeController attitudeController(100);
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
  attitudeController.setGains(YAW, 60, 0, 10);

  attitudeController.setActuationThreshold(PITCH, 0);
  attitudeController.setActuationThreshold(ROLL, 0);
  attitudeController.setActuationThreshold(YAW, 0);

  attitudeController.setDesiredState(0, 0, 0);
  attitudeController.begin();
  attitudeController.enable();
}

void loop() {
  /*
  // Read the desired yaw from the serial port connected
  // to the main flight computer
  if (mySerial.available()) {
    desiredYaw = (long) mySerial.read();
  }
  */

  /*
  // Determine if we should be controlling the attitude
  // of the payload by reading the pin
  if (digitalRead(ATTITUDE_CONTROL_PIN)) {
    attitudeController.enable();
  } else {
    attitudeController.disable();
  }
  */

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
  attitudeController.setDesiredState(0, 0, desiredYaw);
  attitudeController.updateState(pitch, roll, yaw, millis());
  attitudeController.updateErrors();
  attitudeController.updateActuators();

  delay(50);

}
