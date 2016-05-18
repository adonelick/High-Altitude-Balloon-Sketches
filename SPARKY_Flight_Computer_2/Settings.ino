// Written by Andrew Donelick
// adonelick@hmc.edu

void loadSettings()
{
    if (SD.exists("VALUES.bin")) {
        File valuesFile = SD.open("VALUES.bin", FILE_READ);
        byte temp[4];
        
        attitudeControl = (boolean) valuesFile.read();
        manualHeaterControl = (boolean) valuesFile.read();
        heaterOn = (boolean) valuesFile.read();

        // Read in the relayStates setting
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        relayStates = build_uint16(temp);
        relays.switchRelays(relayStates);

        // Read in the desiredYaw setting
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        temp[2] = valuesFile.read();
        temp[3] = valuesFile.read();
        desiredYaw = build_int32(temp);

        // Read in the pGain setting
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        pGain = build_uint16(temp);

        // Read in the iGain setting
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        iGain = build_uint16(temp);

        // Read in the dGain setting
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        dGain = build_uint16(temp);

        valuesFile.close();
    }
}


void saveSettings()
{
    byte values[20];

    // Save whether we are using attitude control or not
    values[0] = attitudeControl;

    // Save if we are using manual or automatic heater control
    values[1] = manualHeaterControl;
    values[2] = heaterOn;

    // Save the current state of the relays
    values[3] = relayStates & 0xFF;
    values[4] = (relayStates >> 8) & 0xFF;

    // Save the current targeted attitude
    values[5] = desiredYaw & 0xFF;
    values[6] = (desiredYaw >> 8) & 0xFF;
    values[7] = (desiredYaw >> 16) & 0xFF;
    values[8] = (desiredYaw >> 24) & 0xFF;
    
    // Save the attitude controller gains
    values[9] = pGain & 0xFF;
    values[10] = (pGain >> 8) & 0xFF;

    values[11] = iGain & 0xFF;
    values[12] = (iGain >> 8) & 0xFF;

    values[13] = dGain & 0xFF;
    values[14] = (dGain >> 8) & 0xFF;

    // Dump the values to disk, overwritting the old settings
    File valuesFile = SD.open("VALUES.bin", FILE_WRITE);
    if (valuesFile) {
        valuesFile.seek(0);
        valuesFile.write(values, 14);
        valuesFile.close();
    }
}


unsigned int build_uint16(byte bytes[])
{
    unsigned int shiftedBits = ((unsigned int) bytes[1]) << 8;
    return ((unsigned int) bytes[0]) | shiftedBits;
}


unsigned int build_int32(byte bytes[])
{
    long shiftedBits1 = ((long) bytes[1]) << 8;
    long shiftedBits2 = ((long) bytes[2]) << 16;
    long shiftedBits3 = ((long) bytes[3]) << 24;
    return ((long) bytes[0]) | shiftedBits1 | shiftedBits2 | shiftedBits3;
}


