// Written by Andrew Donelick
// <adonelick@hmc.edu>
// 17 June 2014

// Defines the commands to be used in the SPARKY balloon system

#define CUTDOWN                     0
#define CHANGE_TRANSMIT_RATE        1   // Expects a transmission value

// Heater commands
#define TURN_HEATER_ON              2
#define TURN_HEATER_OFF             3
#define MANUAL_HEATER_CONTROL       4   // Expects a transmission value

// Attitude control commands
#define TURN_ATTITUDE_CONTROL_ON    5
#define TURN_ATTITUDE_CONTROL_OFF   6
#define SET_YAW                     7   // Expects a transmission value
#define SET_P_GAIN                  8   // Expects a transmission value
#define SET_I_GAIN                  9   // Expects a transmission value
#define SET_D_GAIN                  10  // Expects a transmission value
#define RESET_ATTITUDE_CONTROLLER   11

// Relay controls
#define SWITCH_RELAYS               100 // Expects a transmission value

// Checks whether radio communication is possible
// between the ground and the balloon
#define CHECK_RADIO_CONNECTION  0xFFFF

