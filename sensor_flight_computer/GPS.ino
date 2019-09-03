/**
 * Written by Andrew Donelick
 */


 void get_GPS_data()
 {
    if (GPS_PORT.available() <= 25) {
        byte gps_data[25];
        size_t numBytes = GPS_PORT.readBytes(gps_data, 25);

        if (numBytes > 0)
        {
            year = (uint8_t) gps_data[0];
            month = (uint8_t) gps_data[1];
            date = (uint8_t) gps_data[2];
            hour = (uint8_t) gps_data[3];
            minute = (uint8_t) gps_data[4];
            second = (uint8_t) gps_data[5];

            latitude = buffer_to_int32(&gps_data[6]);
            longitude = buffer_to_int32(&gps_data[10]);
            altitude = buffer_to_int32(&gps_data[14]);
            speed = buffer_to_uint32(&gps_data[18]);
            heading = buffer_to_uint16(&gps_data[22]);
            satellites = (uint8_t) gps_data[24];
        }
    } else {
        while (GPS_PORT.available() > 0)
        {
            GPS_PORT.read();
        }
        
        byte gps_data[25];
        size_t numBytes = GPS_PORT.readBytes(gps_data, 25);

        if (numBytes > 0)
        {
            year = (uint8_t) gps_data[0];
            month = (uint8_t) gps_data[1];
            date = (uint8_t) gps_data[2];
            hour = (uint8_t) gps_data[3];
            minute = (uint8_t) gps_data[4];
            second = (uint8_t) gps_data[5];

            latitude = buffer_to_int32(&gps_data[6]);
            longitude = buffer_to_int32(&gps_data[10]);
            altitude = buffer_to_int32(&gps_data[14]);
            speed = buffer_to_uint32(&gps_data[18]);
            heading = buffer_to_uint16(&gps_data[22]);
            satellites = (uint8_t) gps_data[24];
        }
    }
 }


uint16_t buffer_to_uint16(byte* binary_data)
{    
    union 
    {
        uint16_t value;
        byte bytes[2];
    } converter;

    memcpy(converter.bytes, binary_data, 2);
    return converter.value;
}


int16_t buffer_to_int16(byte* binary_data)
{    
    union 
    {
        int16_t value;
        byte bytes[2];
    } converter;

    memcpy(converter.bytes, binary_data, 2);
    return converter.value;
}


uint32_t buffer_to_uint32(byte* binary_data)
{    
    union 
    {
        uint32_t value;
        byte bytes[4];
    } converter;

    memcpy(converter.bytes, binary_data, 4);
    return converter.value;
}


int32_t buffer_to_int32(byte* binary_data)
{    
    union 
    {
        int32_t value;
        byte bytes[4];
    } converter;

    memcpy(converter.bytes, binary_data, 4);
    return converter.value;
}
