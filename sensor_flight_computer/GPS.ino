/**
 * Written by Andrew Donelick
 */



 void get_GPS_data()
 {
    bool gps_timeout_event = false;
    unsigned long start_time = millis();
    while (true) 
    {
        // Check for a timeout condition first
        if (millis() - start_time > GPS_SERIAL_TIMEOUT)
        {
            gps_timeout_event = true;
            break;
        }
        
        char x = char(GPS_PORT.read());
        if ((x == '\n') && (!GPS_PORT.available()))
        {
            break;
        }
    }

    if (gps_timeout_event) {
        return;
    }
    
    char yearBytes[20];
    char monthBytes[20];
    char dateBytes[20];
    char hourBytes[20];
    char minuteBytes[20];
    char secondBytes[20];
    char latitudeBytes[20];
    char longitudeBytes[20];
    char altitudeBytes[20];
    char speedBytes[20];
    char headingBytes[20];
    char satellitesBytes[20];
    
    unsigned int year_byte_count = read_GPS_data_to_buffer(yearBytes, 20);
    unsigned int month_byte_count = read_GPS_data_to_buffer(monthBytes, 20);
    unsigned int date_byte_count = read_GPS_data_to_buffer(dateBytes, 20);
    unsigned int hour_byte_count = read_GPS_data_to_buffer(hourBytes, 20);
    unsigned int minute_byte_count = read_GPS_data_to_buffer(minuteBytes, 20);
    unsigned int second_byte_count = read_GPS_data_to_buffer(secondBytes, 20);
    unsigned int latitude_byte_count = read_GPS_data_to_buffer(latitudeBytes, 20);
    unsigned int longitude_byte_count = read_GPS_data_to_buffer(longitudeBytes, 20);
    unsigned int altitude_byte_count = read_GPS_data_to_buffer(altitudeBytes, 20);
    unsigned int speed_byte_count = read_GPS_data_to_buffer(speedBytes, 20);
    unsigned int heading_byte_count = read_GPS_data_to_buffer(headingBytes, 20);
    unsigned int satellite_byte_count = read_GPS_data_to_buffer(satellitesBytes, 20);
    
    year = array_to_uint8_t(yearBytes, year_byte_count);
    month = array_to_uint8_t(monthBytes, month_byte_count);
    date = array_to_uint8_t(dateBytes, date_byte_count);
    hour = array_to_uint8_t(hourBytes, hour_byte_count);
    minute = array_to_uint8_t(minuteBytes, minute_byte_count);
    second = array_to_uint8_t(secondBytes, second_byte_count);
    latitude = array_to_int32_t(latitudeBytes, latitude_byte_count);
    longitude = array_to_int32_t(longitudeBytes, longitude_byte_count);
    altitude = array_to_int32_t(altitudeBytes, altitude_byte_count);
    speed = array_to_uint32_t(speedBytes, speed_byte_count);
    heading = array_to_uint16_t(headingBytes, heading_byte_count);
    satellites = array_to_uint8_t(satellitesBytes, satellite_byte_count);
 }


 uint8_t array_to_uint8_t(char* data, unsigned int numBytes)
{
    uint8_t result = 0;
    for (unsigned int i = 0; i < numBytes; ++i)
    {
        result += pow(10, numBytes - i - 1) * (data[i] - '0');
    }
    return result;
}


uint16_t array_to_uint16_t(char* data, unsigned int numBytes)
{
    uint16_t result = 0;
    for (unsigned int i = 0; i < numBytes; ++i)
    {
        result += pow(10, numBytes - i - 1) * (data[i] - '0');
    }
    return result;
}


uint32_t array_to_uint32_t(char* data, unsigned int numBytes)
{
    uint32_t result = 0;
    for (unsigned int i = 0; i < numBytes; ++i)
    {
        result += pow(10, numBytes - i - 1) * (data[i] - '0');
    }
    return result;
}


int32_t array_to_int32_t(char* data, unsigned int numBytes)
{
    int32_t result = 0;
    int32_t multiplier;
    if (data[0] == '-') {
        multiplier = -1;
        numBytes -= 1;
    } else {
        multiplier = 1;
    };

    for (unsigned int i = 0; i < numBytes; ++i)
    {
        if (multiplier == 1) {
            result += pow(10, numBytes - i - 1) * (data[i] - '0');
        } else {
            result += pow(10, numBytes - i - 1) * (data[i+1] - '0');
        }
    }
    return multiplier * result;
}



unsigned int read_GPS_data_to_buffer(char* data_buffer, unsigned int max_length)
{
    unsigned int i = 0;
    
    unsigned long start_time = millis();
    while (true) 
    {
        // Check for a timeout condition first
        if (millis() - start_time > GPS_SERIAL_TIMEOUT)
        {
            break;
        }

        // Now, try to get data from the GPS
        if (GPS_PORT.available() > 0)
        {
            char gps_byte = char(GPS_PORT.read());
            if ((gps_byte == ',') && (i > 0)) {
                break;
            } else if (gps_byte == '\n') {
                break;
            } else if ((gps_byte == ',') && (i == 0)) {
                // pass
            } else {
                data_buffer[i] = gps_byte;
                ++i; 
            }  
        }

        if (i == max_length)
        {
            break;
        }
    }

    return i;
}
