
void buildAckPacket(byte command, unsigned long commandValue)
{
    ackPacket.clear();
    ackPacket.addByte(RADIO_TEST);
    ackPacket.addByte((byte) ((commandValue >> 24) & 0xFF));
    ackPacket.addByte((byte) ((commandValue >> 16) & 0xFF));
    ackPacket.addByte((byte) ((commandValue >> 8) & 0xFF));    
    ackPacket.addByte((byte) (commandValue & 0xFF));
}


void updateDataPacket()
{
    union 
    {
        unsigned long value;
        byte bytes[4];
    } ulongConverter;

    union
    {
        unsigned int value;
        byte bytes[2];
    } uintConverter;
    
    /* Add the time since last data update */
    ulongConverter.value = millis() - lastDataArrival;
    dataPacket.appendBytes(ulongConverter.bytes, 4);

    /* Add the relay states */
    ulongConverter.value = (unsigned long) relayStates;
    dataPacket.appendBytes(ulongConverter.bytes, 4);

    /* Add the checksum */
    uintConverter.value = dataPacket.computeChecksum();
    dataPacket.appendBytes(uintConverter.bytes, 2);
}


