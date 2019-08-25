// Written by Andrew Donelick
// 14 December 2015
// adonelick@hmc.edu


/*
 * This class assists with managing interactions with a relay
 * module. You can turn relays on or off, and determine an 
 * integer which shows the states of all relays.
 */


#ifndef RELAY_MODULE_H
#define RELAY_MODULE_H 1

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif


#define NUM_RELAYS 4

class RelayModule
{
    private:

        bool relayStates_[NUM_RELAYS];
        int relayPins_[NUM_RELAYS];

    public:

        RelayModule(int relayPins[], int numRelayPins);

        // Set the pins used to control the relays as outputs, 
        // intially turn all of the relays off
        void begin();

        // Turn a specific relay on
        void switchRelayOn(int relayIndex);

        // Turn a specific relay off
        void switchRelayOff(int relayIndex);

        // Switch multiple relays on/off using a relayState integer
        // (the bits of the integer control whether a relay is on or off)
        void switchRelays(unsigned int relayState);

        // Get the current relay state integer which represents the current
        // state of all used relays
        unsigned int getRelayStates();

        // Determine whether a specific relay is on or off
        bool getRelayState(int relayIndex);
};


#endif // RelayModule included