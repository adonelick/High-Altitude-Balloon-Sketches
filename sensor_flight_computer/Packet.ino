

/**
 * This function assumes that the given array has room for all of the 
 * stuff we are putting into it. 
 */
unsigned int buildPacket(byte* packet)
{
    unsigned int index = 0;

    /* Add the battery voltage */
    addFloat(packet, batteryVoltage, index);

    /* Insert the temperature readings */
    addFloat(packet, interiorTemperature1, index);
    addFloat(packet, interiorTemperature2, index);
    addFloat(packet, interiorTemperature3, index);
    addFloat(packet, exteriorTemperature, index);

    /* Insert the pressure and humidity readings */
    addFloat(packet, pressure, index);
    addFloat(packet, humidity, index);

    /* Add the GPS data */
    addByte(packet, year, index);
    addByte(packet, month, index);
    addByte(packet, date, index);
    addByte(packet, hour, index);
    addByte(packet, minute, index);
    addByte(packet, second, index);
    addLong(packet, latitude, index);
    addLong(packet, longitude, index);
    addLong(packet, altitude, index);
    addUnsignedLong(packet, speed, index);
    addUnsignedInt(packet, heading, index);
    addByte(packet, satellites, index);
    
    /* Insert the IMU sensor readings */
    addFloat(packet, ax, index);
    addFloat(packet, ay, index);
    addFloat(packet, az, index);
    addFloat(packet, gx, index);
    addFloat(packet, gy, index);
    addFloat(packet, gz, index);
    addFloat(packet, mx, index);
    addFloat(packet, my, index);
    addFloat(packet, mz, index);
    
    /* Insert the attitude data*/
    addFloat(packet, pitch, index);
    addFloat(packet, roll, index);
    addFloat(packet, yaw, index);

    /* Insert the time since the last reset */
    addUnsignedLong(packet, resetTime, index);

    /* Insert the data logging status */
    addBoolean(packet, dataLogging, index);
    addUnsignedInt(packet, filenameIndex, index);

    return index;
}

void addBoolean(byte* packet, boolean x, unsigned int& index)
{
    packet[index++] = (byte) x;
}

void addByte(byte* packet, byte x, unsigned int& index)
{
    packet[index++] = x;
}

void addFloat(byte* packet, float x, unsigned int& index)
{
    union 
    {
        float value;
        byte bytes[4];
    } converter;

    converter.value = x;
    memcpy(packet + index, converter.bytes, 4);
    index += 4;
}

void addUnsignedLong(byte* packet, unsigned long x, unsigned int& index)
{
    union 
    {
        unsigned long value;
        byte bytes[4];
    } converter;

    converter.value = x;
    memcpy(packet + index, converter.bytes, 4);
    index += 4;
}

void addLong(byte* packet, long x, unsigned int& index)
{
    union 
    {
        long value;
        byte bytes[4];
    } converter;

    converter.value = x;
    memcpy(packet + index, converter.bytes, 4);
    index += 4;
}

void addUnsignedInt(byte* packet, unsigned int x, unsigned int& index)
{
    union 
    {
        unsigned int value;
        byte bytes[2];
    } converter;

    converter.value = x;
    memcpy(packet + index, converter.bytes, 2);
    index += 2;
}

void addInt(byte* packet, int x, unsigned int& index)
{
    union 
    {
        int value;
        byte bytes[2];
    } converter;

    converter.value = x;
    memcpy(packet + index, converter.bytes, 2);
    index += 2;
}
