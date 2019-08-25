
#include "Packet.h"


Packet::Packet()
    : numBytes_(0),
      numEscapedBytes_(0)
{
    // Nothing to do here
}


Packet::Packet(byte* bytes, unsigned int numBytes)
    : numBytes_(numBytes),
      numEscapedBytes_(0)
{
    memcpy(bytes_, bytes, numBytes);
    escapeBytes();
}


void Packet::clear()
{
    numBytes_ = 0;
    numEscapedBytes_ = 0;
}


unsigned int Packet::getKISS(byte* kissPacket, byte packetType)
{
    unsigned int index = 0;
    
    /* Start the packet with the KISS bytes */
    kissPacket[index++] = FEND;
    kissPacket[index++] = 0x00;

    /* State the command type */
    kissPacket[index++] = packetType;

    /* Insert the data to report */
    memcpy(kissPacket + index, escapedBytes_, numEscapedBytes_);
    index += numEscapedBytes_;

    /* End the packet with the FEND byte */
    kissPacket[index++] = FEND;
    return index;
}


void Packet::addByte(byte newByte)
{
    bytes_[numBytes_++] = newByte;
}


void Packet::addEscapedByte(byte newByte)
{
    escapedBytes_[numEscapedBytes_++] = newByte;
}


byte Packet::getByte(unsigned int index)
{
    return bytes_[index];
}


void Packet::appendBytes(byte* bytes, unsigned int numBytes)
{
    memcpy(bytes_ + numBytes_, bytes, numBytes);
    numBytes_ += numBytes;
}


void Packet::appendEscapedBytes(byte* escapedBytes, unsigned int numBytes)
{
    memcpy(escapedBytes_ + numEscapedBytes_, escapedBytes, numBytes);
    numEscapedBytes_ += numBytes;
}


unsigned int Packet::getNumBytes()
{
    return numBytes_;
}


unsigned int Packet::computeChecksum()
{
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < numBytes_; i += 2)
    {
        if (i == numBytes_ - 1) {
            checksum += (unsigned int) bytes_[i];
        } else {
            unsigned int x = bytes_[i+1];
            x |= (bytes_[i] << 8);
            checksum += x;
        }
    }

    return 0xFFFF - checksum;
}


void Packet::escapeBytes()
{
    unsigned int j = 0;
    for (unsigned int i = 0; i < numBytes_; ++i)
    {
        if (bytes_[i] == FEND) {
            escapedBytes_[j++] = FESC;
            escapedBytes_[j++] = TFEND;            
        } else if (bytes_[i] == FESC) {
            escapedBytes_[j++] = FESC;
            escapedBytes_[j++] = TFESC;
        } else {
            escapedBytes_[j++] = bytes_[i];
        }
    }

    numEscapedBytes_ = j;
}


void Packet::descapeBytes()
{
    unsigned int i = 0;
    unsigned int j = 0;

    while (i < numEscapedBytes_)
    {
        if ((escapedBytes_[i] == FESC) && (i != numEscapedBytes_ - 1))
        {
            if (escapedBytes_[i+1] == TFEND)
            {
                bytes_[j++] = FEND;
                i++;
            } else if (escapedBytes_[i+1] == TFESC) {
                bytes_[j++] = FESC;
                i++;
            } else {
                bytes_[j++] = escapedBytes_[i];
            }
        } else {
            bytes_[j++] = escapedBytes_[i];
        }

        i++;
    }

    numBytes_ = j;
}
