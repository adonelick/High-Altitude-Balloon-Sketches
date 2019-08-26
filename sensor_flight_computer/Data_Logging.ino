/**
 * Written by Andrew Donelick
 * adonelick@hmc.edu
 */

/**
 * This function writes the header of the data file to 
 * the SD card installed on the sensor computer. 
 * 
 * :param file: Reference to the DataFile object
 * 
 * :return: None
 */
void setupFileEntries(DataFile& file)
{
    /* Set the measurements to be logged in the dataFile */
    file.addEntry("Interior Temperature 1");
    file.addEntry("Interior Temperature 2");
    file.addEntry("Interior Temperature 3");
    file.addEntry("Exterior Temperature");
    file.addEntry("Pressure");
    file.addEntry("Humidity");

    file.addEntry("Acceleration x");
    file.addEntry("Acceleration y");
    file.addEntry("Acceleration z");
    file.addEntry("Rotation Rate x");
    file.addEntry("Rotation Rate y");
    file.addEntry("Rotation Rate z");
    file.addEntry("Magnetic Field x");
    file.addEntry("Magnetic Field y");
    file.addEntry("Magnetic Field z");

    file.addEntry("Pitch");
    file.addEntry("Roll");
    file.addEntry("Yaw");
    
    file.addEntry("GPS Year");
    file.addEntry("GPS Month");
    file.addEntry("GPS Date");
    file.addEntry("GPS Hour");
    file.addEntry("GPS Minute");
    file.addEntry("GPS Second");
    file.addEntry("Latitude");
    file.addEntry("Longitude");
    file.addEntry("Altitude");
    file.addEntry("Speed");
    file.addEntry("Heading");
    file.addEntry("Number of Satellites");

    file.addEntry("Battery Voltage");
    file.addEntry("Radio Transmission");
}


void logData(DataFile& file)
{
    file.writeEntry(interiorTemperature1);
    file.writeEntry(interiorTemperature2);
    file.writeEntry(interiorTemperature3);
    file.writeEntry(exteriorTemperature);
    file.writeEntry(pressure);
    file.writeEntry(humidity);

    file.writeEntry(ax);
    file.writeEntry(ay);
    file.writeEntry(az);
    file.writeEntry(gx);
    file.writeEntry(gy);
    file.writeEntry(gz);
    file.writeEntry(mx);
    file.writeEntry(my);
    file.writeEntry(mz);

    file.writeEntry(pitch);
    file.writeEntry(roll);
    file.writeEntry(yaw);

    file.writeEntry(year);
    file.writeEntry(month);
    file.writeEntry(date);
    file.writeEntry(hour);
    file.writeEntry(minute);
    file.writeEntry(second);
    file.writeEntry(latitude);
    file.writeEntry(longitude);
    file.writeEntry(altitude);
    file.writeEntry(speed);
    file.writeEntry(heading);
    file.writeEntry(satellites);

    file.writeEntry(batteryVoltage);
    file.writeEntry(radioTransmitting);
}
