// Written by Andrew Donelick
// <adonelick@hmc.edu>
// 15 June 2014

// Copyright (c) 2014 Andrew Donelick
// All rights reserved.

/*
 * This library contains code for sending and recieving packets 
 * through a 2-meter radio transciever. The code was developed 
 * for an Alinco DJ-V57T transciever, coupled with Tigertronics 
 * radio modems. The purpose of this library is to facilitate 
 * communication with a high altitude balloon module (send commands
 * and recieve instrument updates) throughout the flight.
 */

#ifndef PACKET_RADIO_H
#define PACKET_RADIO_H 1

#include <inttypes.h>

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif

#define MAX_BUFFER_LENGTH 300

#define TRUE 1
#define FALSE 0

// These constants are used in determining where a communication
// was sent from, and what type of communication it is
#define GROUND 0x0000
#define BALLOON 0xFFFF

#define REPORT 0x1111
#define COMMAND 0x2222
#define COMMAND_RESPONSE 0x3333



class PacketRadio
{
    private:
        uint8_t pinDSR_;
        uint8_t pinRTS_;
        HardwareSerial& radioSerial_;
        HardwareSerial& debugSerial_;
        uint16_t bufferPosition_;
        char buffer_[MAX_BUFFER_LENGTH];
        unsigned long lastTransmissionTime_;
        unsigned long delay_;

    public:
        PacketRadio(HardwareSerial& radioSerial, uint8_t DSR, uint8_t RTS, unsigned long delay);

        // Initializes the radio, prepares it for communication
        void begin();

        // Checks whether there is data available to read from the radio
        bool available();

        // Sets the time between automatic radio transmissions
        void setTransmissionDelay(unsigned long delay);

        // Determines whether or not enough has time has past since the last
        // radio transmission to send another
        bool timeToSendPacket();

        // Sends all the data in the data[] array over the radio link, 
        // along with a checksum to verify the data's integrity
        void sendData(uint16_t data[], uint16_t arraySize);

        // Returns true if the message is recieved (apparently) without any
        // defects. The entire message is returned in data[]
        bool recieveData(char packet[], uint16_t& arraySize);

        // Reads in the recieved packet and extracts all the data values
        // contained within. Also checks the data's integrity with a checksum.
        bool processData(char packet[], uint16_t dataOut[], uint16_t& dataLength);

    private:

        // Computes the checksum of the data[] array
        // The sum of the data and the checksum should be 0xFFFF
        uint16_t computeChecksum(uint16_t data[], uint16_t arraySize);

        // Checks whether the first six letters of the given buffer
        // are the start of a message.
        bool messageStarting(char buffer[], uint16_t index);

        // Checks whether the next six leters of the given buffer
        // are the end of the message.
        bool messageEnding(char buffer[], uint16_t index);

        // Clears the buffer for receiving data
        void clearBuffer();
};


#endif // PACKET_RADIO_H included