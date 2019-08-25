/**
 * Written by Andrew Donelick
 * 24 June 2017
 */

void displayData()
{
    /* Print the battery voltage and data logging */
    Serial.print("Battery Voltage:           ");
    Serial.println(batteryVoltage);
    Serial.print("Data Logging:              ");
    dataLogging ? Serial.println("True") : Serial.println("False");
    Serial.print("Data File:                 ");
    Serial.println(dataFile.getFilename());
    Serial.print("SD Card Present:           ");
    digitalRead(SD_CARD_DETECT) ? Serial.println("True") : Serial.println("False");
    
    /* Print the temperature information */
    Serial.print("Interior Temperature 1:    ");
    Serial.println(interiorTemperature1);
    Serial.print("Interior Temperature 2:    ");
    Serial.println(interiorTemperature2);
    Serial.print("Exterior Temperature:      ");
    Serial.println(exteriorTemperature);

    /* Display the pressure data */
    Serial.print("Pessure (Pa):              ");
    Serial.println(pressure);

    /* Print the humidity data */
    Serial.print("Humidity:                  ");
    Serial.println(humidity);
    
    /* Print the data from the GPS */
    Serial.print("Latitude/Longitude:        ");
    Serial.print(latitude);
    Serial.print(", ");
    Serial.println(longitude);
    Serial.print("Altitude:                  ");
    Serial.println(3.24 * rawAltitude / 100.0);
    Serial.print("Ascent Rate:               ");
    Serial.println(ascentRate);
    Serial.print("Speed:                     ");
    Serial.println(payloadSpeed);

    /* Display the data from the IMU */
    Serial.print("Acceleration (x, y, z):    ");
    Serial.print(ax);
    Serial.print(", ");
    Serial.print(ay);
    Serial.print(", ");
    Serial.println(az);

    Serial.print("Rates (x, y, z):           ");
    Serial.print(gx);
    Serial.print(", ");
    Serial.print(gy);
    Serial.print(", ");
    Serial.println(gz);

    Serial.print("Magnetometer (x, y, z):    ");
    Serial.print(mx);
    Serial.print(", ");
    Serial.print(my);
    Serial.print(", ");
    Serial.println(mz);

    /* Display the attitude data */
    Serial.print("Pitch:                     ");
    Serial.println(pitch);
    Serial.print("Roll:                      ");
    Serial.println(roll);
    Serial.print("Yaw:                       ");
    Serial.println(yaw);
    
    /* Print the binary data packet that is sent to the COMM computer */
    Serial.print("Packet Size:               ");
    Serial.println(numBytes);
    Serial.println("Data Packet: ");
    for (unsigned int i = 0; i < numBytes; ++i)
    {
        if ((i % 40 == 0) && (i != 0)) {
            Serial.println(" ");
        }
        
        if (data[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(data[i], HEX);
    }
    
    /* Print a separation line between data reports */
    Serial.println('\n');
}

