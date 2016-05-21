// Written by Andrew Donelick
// 18 May 2016
// adonelick@hmc.edu

void loadSettings()
{
    // Loads the saved runtime settings from the SD card, and acts
    // upon those loaded settings. This should minimize disruptions of
    // sensitive systems, like attitude control and relay states.
    
    if (SD.exists("VALUES.bin")) {
        File valuesFile = SD.open("VALUES.bin", FILE_READ);
        byte temp[4];

        // Read the old state of the attitude controller, and 
        // turn it on/off as desired
        attitudeControl = (boolean) valuesFile.read();
        if (attitudeControl) {
            digitalWrite(ATTITUDE_PIN, HIGH);
            relays.switchRelayOn(RELAY_3);
        } else {
            digitalWrite(ATTITUDE_PIN, LOW);
            relays.switchRelayOff(RELAY_3);
        }
        
        manualHeaterControl = (boolean) valuesFile.read();
        heaterOn = (boolean) valuesFile.read();

        // Read in the relayStates setting, set the relays as necessary
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        relayStates = build_uint16(temp);
        relays.switchRelays(relayStates);

        // Read in the desiredYaw setting, send the desired yaw to the attitude controller
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        temp[2] = valuesFile.read();
        temp[3] = valuesFile.read();
        desiredYaw = build_int32(temp);
        attitudeSerial.write('Y');
        sendToAttitudeController(desiredYaw);

        // Read in the pGain setting, send the desired yaw to the attitude controller
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        pGain = build_uint16(temp);
        attitudeSerial.write('P');
        sendToAttitudeController((long) pGain);

        // Read in the iGain setting, send the desired yaw to the attitude controller
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        iGain = build_uint16(temp);
        attitudeSerial.write('I');
        sendToAttitudeController((long) iGain);

        // Read in the dGain setting, send the desired yaw to the attitude controller
        temp[0] = valuesFile.read();
        temp[1] = valuesFile.read();
        dGain = build_uint16(temp);
        attitudeSerial.write('D');
        sendToAttitudeController((long) dGain);

        valuesFile.close();
    }
}


void saveSettings()
{
    // Dump the sensitive runtime settings to a file. This enables them
    // to be reloaded in the event that the system is reset while it is
    // running. Ideally, this should lead to minimized any disruptions to 
    // attitude control, heating, relay states, etc in the event of a 
    // hardware reset. 
    byte values[20];

    // Save whether we are using attitude control or not
    values[0] = attitudeControl;

    // Save if we are using manual or automatic heater control
    values[1] = manualHeaterControl;
    values[2] = heaterOn;

    // Save the current state of the relays
    values[3] = (relayStates >> 8) & 0xFF; 
    values[4] = relayStates & 0xFF;

    // Save the current targeted attitude
    values[5] = (desiredYaw >> 24) & 0xFF;
    values[6] = (desiredYaw >> 16) & 0xFF;
    values[7] = (desiredYaw >> 8) & 0xFF;
    values[8] = desiredYaw & 0xFF;
    
    // Save the attitude controller gains
    values[9] = (pGain >> 8) & 0xFF;
    values[10] = pGain & 0xFF;
    
    values[11] = (iGain >> 8) & 0xFF;
    values[12] = iGain & 0xFF;

    values[13] = (dGain >> 8) & 0xFF;
    values[14] = dGain & 0xFF;
    
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
    unsigned int shiftedBits = ((unsigned int) bytes[0]) << 8;
    return ((unsigned int) bytes[1]) | shiftedBits;
}


unsigned int build_int32(byte bytes[])
{
    // Converts an array of four bytes into a signed 32 bit integer
    long shiftedBits1 = ((long) bytes[2]) << 8;
    long shiftedBits2 = ((long) bytes[1]) << 16;
    long shiftedBits3 = ((long) bytes[0]) << 24;
    return ((long) bytes[3]) | shiftedBits1 | shiftedBits2 | shiftedBits3;
}


void sendToAttitudeController(long command)
{
    // Sends a long value to the attitude controllers
    attitudeSerial.write((char) ((command >> 24)) & 0xFF);
    attitudeSerial.write((char) ((command >> 16)) & 0xFF);
    attitudeSerial.write((char) ((command >> 8)) & 0xFF);
    attitudeSerial.write((char) (command & 0xFF));
}


