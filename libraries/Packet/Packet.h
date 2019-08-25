

#ifndef PACKET_H
#define PACKET_H 1

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif


#define PACKET_SIZE 500

#define FEND 0xC0
#define FESC 0xDB
#define TFEND 0xDC
#define TFESC 0xDD

#define DATA 0
#define RADIO_COMMAND 1
#define ACK 2


class Packet
{

    private:

        byte bytes_[PACKET_SIZE];
        byte escapedBytes_[PACKET_SIZE];
        unsigned int numBytes_;
        unsigned int numEscapedBytes_;

    public:

        Packet();

        Packet(byte* bytes, unsigned int numBytes);

        unsigned int getKISS(byte* kissPacket, byte packetType);

        void addByte(byte newByte);

        void addEscapedByte(byte newByte);

        byte getByte(unsigned int index);

        void appendBytes(byte* bytes, unsigned int numBytes);

        void appendEscapedBytes(byte* escapedBytes, unsigned int numBytes);

        void clear();

        unsigned int getNumBytes();

        void escapeBytes();

        void descapeBytes();

        unsigned int computeChecksum();

};


#endif