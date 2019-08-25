void setupFileEntries(DataFile& file)
{
    /* Set the measurements to be logged in the dataFile */
    file.addEntry("Interior Temperature 1");
    file.addEntry("Interior Temperature 2");
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
    
    file.addEntry("GPS Time");
    file.addEntry("GPS Sentences");
    file.addEntry("GPS Failed Sentences");
    file.addEntry("Number of Satellites");
    file.addEntry("Latitude");
    file.addEntry("Longitude");
    file.addEntry("Altitude");
    file.addEntry("Ascent Rate");
    file.addEntry("Speed");

    file.addEntry("Battery Voltage");
    file.addEntry("Radio Transmission");
}


void logData(DataFile& file)
{
    file.writeEntry(interiorTemperature1);
    file.writeEntry(interiorTemperature2);
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

    file.writeEntry(gpsTime);
    file.writeEntry(gpsSentences);
    file.writeEntry(failedSentences);
    file.writeEntry(numSatellites);
    file.writeEntry(latitude);
    file.writeEntry(longitude);
    file.writeEntry(rawAltitude / 100.0);
    file.writeEntry(ascentRate);
    file.writeEntry(payloadSpeed);

    file.writeEntry(batteryVoltage);
    file.writeEntry(radioTransmitting);
}

