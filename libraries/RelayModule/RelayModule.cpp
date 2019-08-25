// Written by Andrew Donelick
// 14 December 2015
// adonelick@hmc.edu

#include "RelayModule.h"

RelayModule::RelayModule(int relayPins[], int numRelayPins)
{
    for (int i = 0; i < NUM_RELAYS; ++i)
    {
        relayStates_[i] = false;
        relayPins_[i] = 0;
    }

    for (int i = 0; i < numRelayPins; ++i)
    {
        relayPins_[i] = relayPins[i];
    }
}


void RelayModule::begin()
{
    for (int i = 0; i < NUM_RELAYS; ++i)
    {
        pinMode(relayPins_[i], OUTPUT);
        digitalWrite(relayPins_[i], HIGH);
    }
}


void RelayModule::switchRelayOn(int relayIndex)
{
    relayStates_[relayIndex] = true;
    digitalWrite(relayPins_[relayIndex], LOW);
}


void RelayModule::switchRelayOff(int relayIndex)
{
    relayStates_[relayIndex] = false;
    digitalWrite(relayPins_[relayIndex], HIGH);
}


void RelayModule::switchRelays(unsigned int relayStates)
{
    bool desiredRelayState = false;

    for (int i = 0; i < NUM_RELAYS; ++i)
    {
        desiredRelayState = (relayStates & (0x0001 << i)) ? true : false;

        if (desiredRelayState) {
            switchRelayOn(i);
        } else {
            switchRelayOff(i);
        }

    }
}

unsigned int RelayModule::getRelayStates()
{
    unsigned int state = relayStates_[0];
    for (int i = 1; i < NUM_RELAYS; ++i)
    {
        state = state | (((unsigned int) relayStates_[i]) << i);
    }

    return state;
}

bool RelayModule::getRelayState(int relayIndex)
{
    return relayStates_[relayIndex];
}
