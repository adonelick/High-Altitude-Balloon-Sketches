// Written by Andrew Donelick
// <adonelick@hmc.edu>
// 15 June 2014

// Copyright (c) 2014 Andrew Donelick
// All rights reserved.

#include "PacketRadio.h"

PacketRadio::PacketRadio(HardwareSerial& radioSerial, uint8_t DSR, uint8_t RTS, unsigned long delay)
    : radioSerial_(radioSerial),
      pinDSR_(DSR),
      pinRTS_(RTS),
      debugSerial_(Serial),
      lastTransmissionTime_(0),
      delay_(delay),
      bufferPosition_(0)
{
    // Nothing else to do here...
}

void PacketRadio::begin()
{
    radioSerial_.begin(1200);
    pinMode(pinDSR_, INPUT);
    pinMode(pinRTS_, OUTPUT);
    digitalWrite(pinRTS_, LOW);
    //delay(500);
    //digitalWrite(pinRTS_, LOW);
    clearBuffer();
}

bool PacketRadio::available()
{
    while (radioSerial_.available()) {
        buffer_[bufferPosition_] = radioSerial_.read();
        ++bufferPosition_;
    }

    return digitalRead(pinDSR_) && (bufferPosition_ > 0);
}

void PacketRadio::setTransmissionDelay(unsigned long delay)
{
    delay_ = delay;
}

bool PacketRadio::timeToSendPacket()
{
    return (millis() - lastTransmissionTime_) >= delay_;
}

void PacketRadio::sendData(uint16_t data[], uint16_t arraySize)
{
    uint16_t checkSum = computeChecksum(data, arraySize);

    // Start the radio communication (key the mic)
    digitalWrite(pinRTS_, HIGH);
    delay(1000);

    // Start writing data to the serial port
    radioSerial_.write("KF7YUR");

    for (int index = 0; index < arraySize; ++index)
    {
        // Send the first (most significant bits) first, 
        // followed by the least significant bits
        radioSerial_.write(data[index] >> 8);
        radioSerial_.write(data[index]);
    }

    radioSerial_.write(checkSum >> 8);
    radioSerial_.write(checkSum);
    
    radioSerial_.write("SPARKY");
    radioSerial_.flush();
    delay(1000);
    digitalWrite(pinRTS_, LOW);

    lastTransmissionTime_ = millis();
}

bool PacketRadio::recieveData(char packet[], uint16_t& arraySize)
{
    // This function assumes that data is available to read,
    // that is, DSR is reading HIGH

    // Read all the available bytes from the radio's serial port,
    // and store them in the temporary buffer
    while (radioSerial_.available() && bufferPosition_ < MAX_BUFFER_LENGTH)
    {
        buffer_[bufferPosition_] = radioSerial_.read();
        ++bufferPosition_;
    }

    // Get the index of the start of the real message
    uint16_t i = 0;
    while (!messageStarting(buffer_, i) && i < bufferPosition_)
    {
        ++i;
    }

    // Loop throught the buffer and extract the entire message
    char message[2*MAX_BUFFER_LENGTH];
    uint16_t index = 0;
    while (!messageEnding(buffer_, i))
    {
        message[index] = buffer_[i];
        ++index;
        ++i;

        if (i > 2*MAX_BUFFER_LENGTH) {
            break;
        }
    }

    // Read in the last 6 bytes of the message (should be SPARKY)
    if (i < 2*MAX_BUFFER_LENGTH) {
        for (uint8_t x = 0; x < 6; ++x)
        {
            message[index] = buffer_[i];
            ++index;
            ++i;
        }
    }

    // We now have the entire message stored in the "message" variable.
    // Check if the start and end of the message variable appear as 
    // they should and return the message.
    
    arraySize = index;

    for (i = 0; i < index; ++i)
    {
        packet[i] = message[i];
    }

    clearBuffer();
    return messageStarting(message, 0) && messageEnding(message, index - 6);
}

// This function assumes that a reasonale packet (starts with KF7YUR)
// has just come through, and needs to be parsed.
bool PacketRadio::processData(char packet[], uint16_t dataOut[], uint16_t& dataLength)
{
    // Skip the first 6 characters (the KF7YUR part)

    uint16_t packetIndex = 6;
    uint16_t dataIndex = 0;

    uint16_t element1;
    uint16_t element2;

    while (!messageEnding(packet, packetIndex) && dataIndex < MAX_BUFFER_LENGTH)
    {
        // element1 is the most significant bits
        // element2 is the least significant bits
        element1 = (uint16_t) packet[packetIndex];
        element2 = (uint16_t) packet[packetIndex + 1];

        // Glue the two bytes together to form a single 
        // unsigned integer value
        dataOut[dataIndex] = (element1 << 8) | (element2 & 0x00FF);

        packetIndex += 2;
        ++dataIndex;
    }

    dataLength = dataIndex;

    uint16_t sum = 0;
    for (uint16_t i = 0; i < dataIndex; ++i)
    {
        sum += dataOut[i];
    }

    return (sum == 0xFFFF);
}


///////////////////////////////////////////////////////////////////////////////
//////////////////////////  HELPER FUNCTIONS  /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


uint16_t PacketRadio::computeChecksum(uint16_t data[], uint16_t arraySize)
{
    uint16_t sum = 0;

    for (int index = 0; index < arraySize; ++index)
    {
        sum += data[index];
    }

    return 0xFFFF - sum;
}

bool PacketRadio::messageStarting(char buffer[], uint16_t index)
{
    bool test1 = (buffer[index] == 'K');
    bool test2 = (buffer[index + 1] == 'F');
    bool test3 = (buffer[index + 2] == '7');
    bool test4 = (buffer[index + 3] == 'Y');
    bool test5 = (buffer[index + 4] == 'U');
    bool test6 = (buffer[index + 5] == 'R');

    return (test1 && test2 && test3 && test4 && test5 && test6);
}

bool PacketRadio::messageEnding(char buffer[], uint16_t index)
{
    bool test1 = (buffer[index] == 'S');
    bool test2 = (buffer[index + 1] == 'P');
    bool test3 = (buffer[index + 2] == 'A');
    bool test4 = (buffer[index + 3] == 'R');
    bool test5 = (buffer[index + 4] == 'K');
    bool test6 = (buffer[index + 5] == 'Y');

    return (test1 && test2 && test3 && test4 && test5 && test6);
}

void PacketRadio::clearBuffer()
{
    for (uint16_t i = 0; i < MAX_BUFFER_LENGTH; ++i) {
        buffer_[i] = 0;
    }
    bufferPosition_ = 0;
}
