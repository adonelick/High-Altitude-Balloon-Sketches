// Written by Andrew Donelick
// 17 December 2015

// This file contains functions which deal with transmitting status
// packets, receiving commands, echoing a received packet, etc.


void buildStatusPacket()
{
    data[0] = BALLOON; // Source
    data[1] = REPORT; // Packet type
    data[2] = NULL; // Not a command, so no command
    data[3] = interiorTemp1; // signed
    data[4] = interiorTemp2; // signed
    data[5] = heaterTemp; // signed
    data[6] = exteriorTemp; // signed
    data[7] = humidity; // unsigned
    data[8] = pressure; // unsigned
    data[9] = pitch; // signed
    data[10] = roll; // signed
    data[11] = yaw; // signed
    data[12] = (unsigned int) gps.altitude.meters();
    data[13] = (unsigned int) heaterOn; // unsigned
    data[14] = (unsigned int) dataLogging; // unsigned
    data[15] = (unsigned int) manualHeaterControl; //unsigned
    data[16] = (unsigned int) turbulence; // unsigned
    data[17] = relayStates; // unsigned   
}


void sendStatusPacket()
{
    if (radio.timeToSendPacket())
    {
        radio.sendData(data, 18);
        
        // This delay is included to allow any scrambled sensor 
        // readings caused by the high power radio transmission 
        // to settle down.
        delay(1000);
    }
}


void echoCommand(unsigned int command, unsigned int commandValue)
{
    data[0] = BALLOON; // Source
    data[1] = COMMAND_RESPONSE; // Packet type
    data[2] = command; // Command
    data[3] = commandValue; // Value
    radio.sendData(data, 4);
    delay(1000);
}


void processCommand(unsigned int command, unsigned int commandValue)
{
    switch (command) 
    {
        case CHANGE_TRANSMIT_RATE:
            // Changes the rate at which data is sent to the ground
            radio.setTransmissionDelay((unsigned long) commandValue);
            break;
        case MANUAL_HEATER_CONTROL:
            // Enables (or disables) manual control of heater
            manualHeaterControl = (commandValue) ? true : false;
            break;
        case TURN_HEATER_ON:
            heaterOn = true;
            break;
        case TURN_HEATER_OFF:
            heaterOn = false;
            break;
        case SWITCH_RELAYS:
            relays.switchRelays(commandValue);
            break;
        case CHECK_RADIO_CONNECTION:
            // The connection is already echoed
            // If any other action are required to do the 
            // pre-flight radio check, put them here
            break;
        default:
            // In the default, the command is not recognized.
            break;
    }
}
