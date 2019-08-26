/**
 * Written by Andrew Donelick
 * 24 June 2017
 */

void displayData()
{
    /* Print the battery voltage and data logging */
    DEBUG_PORT.print("Battery Voltage:           ");
    DEBUG_PORT.println(batteryVoltage);
    DEBUG_PORT.print("Data Logging:              ");
    dataLogging ? DEBUG_PORT.println("True") : DEBUG_PORT.println("False");
    DEBUG_PORT.print("Data File:                 ");
    DEBUG_PORT.println(dataFile.getFilename());
    DEBUG_PORT.print("SD Card Present:           ");
    digitalRead(SD_CARD_DETECT) ? DEBUG_PORT.println("True") : DEBUG_PORT.println("False");
    
    /* Print the temperature information */
    DEBUG_PORT.print("Interior Temperature 1:    ");
    DEBUG_PORT.println(interiorTemperature1);
    DEBUG_PORT.print("Interior Temperature 2:    ");
    DEBUG_PORT.println(interiorTemperature2);
    DEBUG_PORT.print("Interior Temperature 3:    ");
    DEBUG_PORT.println(interiorTemperature3);
    DEBUG_PORT.print("Exterior Temperature:      ");
    DEBUG_PORT.println(exteriorTemperature);

    /* Display the pressure data */
    DEBUG_PORT.print("Pessure (Pa):              ");
    DEBUG_PORT.println(pressure);

    /* Print the humidity data */
    DEBUG_PORT.print("Humidity:                  ");
    DEBUG_PORT.println(humidity);
    
    /* Print the data from the GPS */
    DEBUG_PORT.print("Latitude:                  ");
    DEBUG_PORT.println(latitude * 1.0e-7);
    DEBUG_PORT.print("Longitude:                  ");
    DEBUG_PORT.println(longitude * 1.0e-7);
    DEBUG_PORT.print("Altitude:                  ");
    DEBUG_PORT.println(cm_to_ft((double) altitude));
    DEBUG_PORT.print("GPS Update Time:           ");
    DEBUG_PORT.print(hour);
    DEBUG_PORT.print(':');
    DEBUG_PORT.print(minute);
    DEBUG_PORT.print(':');
    DEBUG_PORT.println(second);
    
    /* Display the data from the IMU */
    DEBUG_PORT.print("Acceleration (x, y, z):    ");
    DEBUG_PORT.print(ax);
    DEBUG_PORT.print(", ");
    DEBUG_PORT.print(ay);
    DEBUG_PORT.print(", ");
    DEBUG_PORT.println(az);

    DEBUG_PORT.print("Rates (x, y, z):           ");
    DEBUG_PORT.print(gx);
    DEBUG_PORT.print(", ");
    DEBUG_PORT.print(gy);
    DEBUG_PORT.print(", ");
    DEBUG_PORT.println(gz);

    DEBUG_PORT.print("Magnetometer (x, y, z):    ");
    DEBUG_PORT.print(mx);
    DEBUG_PORT.print(", ");
    DEBUG_PORT.print(my);
    DEBUG_PORT.print(", ");
    DEBUG_PORT.println(mz);

    /* Display the attitude data */
    DEBUG_PORT.print("Pitch:                     ");
    DEBUG_PORT.println(pitch);
    DEBUG_PORT.print("Roll:                      ");
    DEBUG_PORT.println(roll);
    DEBUG_PORT.print("Yaw:                       ");
    DEBUG_PORT.println(yaw);
    
    /* Print the binary data packet that is sent to the COMM computer */
    DEBUG_PORT.print("Packet Size:               ");
    DEBUG_PORT.println(numBytes);
    DEBUG_PORT.println("Data Packet: ");
    for (unsigned int i = 0; i < numBytes; ++i)
    {
        if ((i % 40 == 0) && (i != 0)) {
            DEBUG_PORT.println(" ");
        }
        
        if (data[i] < 0x10) {
            DEBUG_PORT.print("0");
        }
        DEBUG_PORT.print(data[i], HEX);
    }
    
    /* Print a separation line between data reports */
    DEBUG_PORT.println('\n');
}
